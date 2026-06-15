
#include "rendering/vulkan/vulkan_renderer.h"

#include "engine/engine.h"
#include <cstring>

Vulkan::AllocatedBuffer Vulkan::Renderer::allocate_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
	// allocate buffer
	VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	Vulkan::AllocatedBuffer new_buffer;

	// allocate the buffer
	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &new_buffer.buffer, &new_buffer.allocation,
	                         &new_buffer.info));

	return new_buffer;
}

void Vulkan::Renderer::destroy_buffer(const Vulkan::AllocatedBuffer &buffer) {
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

// run a function on the GPU and block on the CPU until it has finished, using the
// dedicated immediate command buffer + fence. This is the mechanism the tutorial
// relies on to perform the staging-buffer copy synchronously.
void Vulkan::Renderer::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function) {
	VK_CHECK(vkResetFences(device.vk_device, 1, &imm_fence));
	VK_CHECK(vkResetCommandBuffer(imm_command_buffer, 0));

	VkCommandBuffer cmd = imm_command_buffer;

	VkCommandBufferBeginInfo cmdBeginInfo = Vulkan::init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = Vulkan::init::command_buffer_submit_info(cmd);
	VkSubmitInfo2 submit = Vulkan::init::submit_info(&cmdinfo, nullptr, nullptr);

	// submit command buffer to the queue and execute it. imm_fence will now block
	// until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(device.graphics_queue, 1, &submit, imm_fence));
	VK_CHECK(vkWaitForFences(device.vk_device, 1, &imm_fence, true, 9999999999));
}

Vulkan::GPUMeshBuffers Vulkan::Renderer::upload_mesh(std::span<uint32_t> indices, std::span<MeshVertex> vertices) {
	const size_t vertexBufferSize = vertices.size() * sizeof(MeshVertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers new_buffer;

	// create vertex buffer
	new_buffer.vertex_buffer = allocate_buffer(vertexBufferSize,
	                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	                                           VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo device_address_info{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
	    .buffer = new_buffer.vertex_buffer.buffer};
	new_buffer.vertex_buffer_address = vkGetBufferDeviceAddress(device.vk_device, &device_address_info);

	// create index buffer
	new_buffer.index_buffer = allocate_buffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	                                          VMA_MEMORY_USAGE_GPU_ONLY);

	// With the buffers allocated, we need to write the data into them. For that, we will be using a staging buffer.
	// This is a very common pattern with vulkan. As GPU_ONLY memory cant be written on CPU, we first write the
	// memory on a temporal staging buffer that is CPU writeable, and then execute a copy command to copy this buffer
	// into the GPU buffers. Its not necesary for meshes to use GPU_ONLY vertex buffers, but its highly recommended
	// unless its something like a CPU side particle system or other dynamic effects.

	// We first create the staging buffer, which will be 1 buffer for both of the copies to index and vertex buffers.
	// Its memory type is CPU_ONLY, and its usage flag is VK_BUFFER_USAGE_TRANSFER_SRC_BIT as the only thing we will
	// do with it is a copy command.
	AllocatedBuffer staging = allocate_buffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	// Once we have the buffer, we can take its mapped adress with GetMappedData(), this gives us a void* pointer we
	// can write to. So we do 2 memcpy commands to copy both spans into it. (VMA keeps the buffer persistently mapped
	// because we allocate with VMA_ALLOCATION_CREATE_MAPPED_BIT, so the pointer lives in the allocation info.)
	void *data = staging.info.pMappedData;

	// copy vertex buffer
	memcpy(data, vertices.data(), vertexBufferSize);
	// copy index buffer
	memcpy((char *)data + vertexBufferSize, indices.data(), indexBufferSize);

	// With the staging buffer written, we run an immediate_submit to run a GPU side command to perform this copy. The
	// command will run 2 VkCmdCopyBuffer commands, which are roughly the same as a memcpy but done by the GPU. You can
	// see how the VkBufferCopy structures mirror directly the memcpys we did to write the staging buffer.
	immediate_submit([&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{0};
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, new_buffer.vertex_buffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{0};
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, new_buffer.index_buffer.buffer, 1, &indexCopy);
	});

	// Once the immediate submit is done, we can safely dispose of the staging buffer and delete it.
	destroy_buffer(staging);

	// Note that this pattern is not very efficient, as we are waiting for the GPU command to fully execute before
	// continuing with our CPU side logic. This is something people generally put on a background thread, whose sole
	// job is to execute uploads like this one, and deleting/reusing the staging buffers.
	return new_buffer;
}

// Walks the engine resource bank and uploads to the GPU any mesh that doesn't yet
// have buffers. gpu_meshes is kept index-aligned with resources.meshes, so a
// MeshComponent::mesh_index indexes straight into both.
void Vulkan::Renderer::upload_pending_meshes() {
	std::vector<MeshGeometry> &meshes = engine->resources.meshes;

	for (size_t i = gpu_meshes.size(); i < meshes.size(); i++) {
		MeshGeometry &geometry = meshes[i];
		gpu_meshes.push_back(upload_mesh(geometry.indices, geometry.vertices));
	}
}
