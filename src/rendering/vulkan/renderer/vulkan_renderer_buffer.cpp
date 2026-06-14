
#include "rendering/vulkan/vulkan_renderer.h"

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

	return new_buffer;
}