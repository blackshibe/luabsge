#include "rendering/vulkan/base/vulkan_types.h"
#include "rendering/vulkan/pipeline/vulkan_pipeline.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "include/imgui/imgui_impl_vulkan.h"

#include "ecs/instance.h"
#include "ecs/scene_camera.h"
#include "ecs/scene_light.h"
#include "ecs/scene_material.h"
#include "ecs/scene_mesh.h"
#include "engine/engine.h"
#include "util/output.h"
#include "vulkan/vulkan_core.h"

#include <cmath>
#include <cstring>
#include <unordered_map>
#include <vector>

static Output output;

void Vulkan::Renderer::draw() {

	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	// (in nanoseconds). Fences have to be reset between uses.
	VK_CHECK(vkWaitForFences(device.vk_device, 1, &get_current_frame()._renderFence, true, 1000000000));

	// the fence is signalled, so the gpu finished this frame's work: safe to flush any
	// per-frame resources now.
	get_current_frame()._deletionQueue.flush();
	get_current_frame().frame_descriptors.clear_pools(device.vk_device);

	VK_CHECK(vkResetFences(device.vk_device, 1, &get_current_frame()._renderFence));

	// request image from the swapchain. We send the _swapchainSemaphore so we can
	// sync our render commands to the swapchain image being ready.
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device.vk_device, swapchain.swapchain, 1000000000,
	                               get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex));

	VkCommandBuffer vk_command_buffer = get_current_frame()._mainCommandBuffer;

	// now that the commands finished executing (we waited on the fence), we can
	// safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(vk_command_buffer, 0));

	// the draw region matches our off-swapchain draw image
	draw_extent.width = draw_image.extent.width;
	draw_extent.height = draw_image.extent.height;

	// we will use this command buffer exactly once, so tell vulkan that for a
	// possible small speedup in command encoding.
	VkCommandBufferBeginInfo command_buffer_begin_info = Vulkan::init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(vk_command_buffer, &command_buffer_begin_info));

	// the compute shader we run for the background needed to draw into GENERAL image
	// layout, but when doing geometry rendering, we need to use COLOR_ATTACHMENT_OPTIMAL.
	// It is possible to draw into GENERAL layout with graphics pipelines, but its lower
	// performance and the validation layers will complain.

	// TODO BETTER SEGMENTATION ---- PREPASS PIPELINES ----------------------------------------------------------------------
	// output.trace("start depth prepass");
	this->pipelines->prepass_depth.bind_for_render(vk_command_buffer, draw_extent);
	this->draw_geometry(vk_command_buffer, this->pipelines->prepass_depth);

	// output.trace("start albedo prepass");
	this->pipelines->prepass_albedo.bind_for_render(vk_command_buffer, draw_extent);
	this->draw_geometry(vk_command_buffer, this->pipelines->prepass_albedo);

	// LIGHTING PIPELINE ---- LIGHTING PIPELINE ----------------------------------------------------------------------
	// the compute shader samples the albedo prepass (binding 0) and the depth prepass
	// (binding 1) and writes the result into draw_image (binding 2, a storage image).
	// Move the sampled images to SHADER_READ_ONLY and the draw image to GENERAL.
	this->pipelines->prepass_albedo.transition_color_image(vk_command_buffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	this->pipelines->prepass_depth.transition_depth_image(vk_command_buffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Vulkan::image::transition_image(vk_command_buffer, draw_image.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	this->draw_light_compute(vk_command_buffer, this->pipelines->pass_lighting);

	// PRESENT IMAGE --------------------------------------------------------------------------------------------------

	// transition the draw image (written by the lighting compute pass) and the swapchain
	// image into their transfer layouts, then copy the draw image into the swapchain image.
	Vulkan::image::transition_image(vk_command_buffer, draw_image.vk_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::image::transition_image(vk_command_buffer, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// move it to swapchain
	Vulkan::image::copy_image_to_image(vk_command_buffer, draw_image.vk_image, swapchain.images[swapchainImageIndex], draw_extent, swapchain.extent);

	// draw imgui directly onto the swapchain image. We need it in color-attachment
	// layout for dynamic rendering, then transition to present.
	Vulkan::image::transition_image(vk_command_buffer, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	draw_imgui(vk_command_buffer, swapchain.image_views[swapchainImageIndex]);

	// make the swapchain image into presentable mode. PRESENT_SRC_KHR is the only
	// layout the swapchain allows for presenting to screen.
	Vulkan::image::transition_image(vk_command_buffer, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(vk_command_buffer));

	// prepare the submission to the queue. We wait on the _swapchainSemaphore, as it
	// is signaled when the swapchain image is ready, and we signal the _renderSemaphore
	// to indicate that rendering has finished.
	VkCommandBufferSubmitInfo cmdinfo = Vulkan::init::command_buffer_submit_info(vk_command_buffer);
	VkSemaphoreSubmitInfo waitInfo =
	    Vulkan::init::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo =
	    Vulkan::init::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, render_semaphores[swapchainImageIndex]);
	VkSubmitInfo2 submit = Vulkan::init::submit_info(&cmdinfo, &signalInfo, &waitInfo);

	// submit command buffer to the queue and execute it. _renderFence will now block
	// until the graphic commands finish execution. This is how we sync gpu to cpu.
	VK_CHECK(vkQueueSubmit2(device.graphics_queue, 1, &submit, get_current_frame()._renderFence));

	// prepare present. This puts the image we just rendered into the visible window.
	// We wait on the _renderSemaphore so drawing finishes before the image is shown.
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swapchain.swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &render_semaphores[swapchainImageIndex];
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(device.graphics_queue, &presentInfo));

	// increase the number of frames drawn
	frame_number++;
}

void Vulkan::Renderer::draw_compute(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline) {
	// bind the compute pipeline and the descriptor set holding its input/output images,
	// then dispatch. The shader uses a 16x16 workgroup, so we divide the draw resolution
	// by 16 and round up.
	vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_pipeline);

	VkDescriptorSet image_set = get_current_frame().frame_descriptors.allocate(device.vk_device, pipeline.vk_descriptor_layout);
	DescriptorWriter writer;
	for (auto &binding : pipeline.bindings)
		writer.write_image(binding.binding, binding.image->vk_view, binding.sampler, binding.layout, binding.type);
	writer.update_set(device.vk_device, image_set);

	vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_layout, 0, 1, &image_set, 0, nullptr);
	vkCmdDispatch(vk_buffer, (uint32_t)std::ceil(draw_extent.width / 16.0), (uint32_t)std::ceil(draw_extent.height / 16.0), 1);
}

void Vulkan::Renderer::draw_light_compute(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline) {
	vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_pipeline);

	// gather every directional light in the scene into a host-visible storage buffer the
	// lighting compute shader reads. The buffer is transient: it is recreated each frame
	// and freed once the frame's fence signals.
	std::vector<Vulkan::GPUDirectionalLight> lights;
	auto light_view = engine->registry.view<Ecs::InstanceComponent, Ecs::DirectionalLightComponent>();
	for (entt::entity entity : light_view) {
		const Ecs::DirectionalLightComponent &light = light_view.get<Ecs::DirectionalLightComponent>(entity);
		lights.push_back(Vulkan::GPUDirectionalLight{
		    .direction = glm::vec4(light.direction, 0.0f),
		    .color = glm::vec4(light.color, 0.0f)});
	}

	int light_count = (int)lights.size();
	size_t buffer_size = (light_count > 0 ? light_count : 1) * sizeof(Vulkan::GPUDirectionalLight);
	Vulkan::AllocatedBuffer light_buffer = allocate_buffer(buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (light_count > 0)
		memcpy(light_buffer.info.pMappedData, lights.data(), light_count * sizeof(Vulkan::GPUDirectionalLight));

	get_current_frame()._deletionQueue.push_function([this, light_buffer]() { destroy_buffer(light_buffer); });

	// same transient-buffer pattern for the scene's point lights
	std::vector<Vulkan::GPUPointLight> point_lights;
	auto point_light_view = engine->registry.view<Ecs::InstanceComponent, Ecs::PointLightComponent>();
	for (entt::entity entity : point_light_view) {
		const Ecs::PointLightComponent &light = point_light_view.get<Ecs::PointLightComponent>(entity);
		point_lights.push_back(Vulkan::GPUPointLight{
		    .position = glm::vec4(light.position, 1.0f),
		    .color = glm::vec4(light.color, 0.0f)});
	}

	int point_light_count = (int)point_lights.size();
	size_t point_buffer_size = (point_light_count > 0 ? point_light_count : 1) * sizeof(Vulkan::GPUPointLight);
	Vulkan::AllocatedBuffer point_light_buffer = allocate_buffer(point_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (point_light_count > 0)
		memcpy(point_light_buffer.info.pMappedData, point_lights.data(), point_light_count * sizeof(Vulkan::GPUPointLight));

	get_current_frame()._deletionQueue.push_function([this, point_light_buffer]() { destroy_buffer(point_light_buffer); });

	VkDescriptorSet image_set = get_current_frame().frame_descriptors.allocate(device.vk_device, pipeline.vk_descriptor_layout);
	DescriptorWriter writer;
	for (auto &binding : pipeline.bindings)
		writer.write_image(binding.binding, binding.image->vk_view, binding.sampler, binding.layout, binding.type);
	writer.write_buffer(3, light_buffer.buffer, buffer_size, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	writer.write_buffer(4, point_light_buffer.buffer, point_buffer_size, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	writer.update_set(device.vk_device, image_set);

	vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_layout, 0, 1, &image_set, 0, nullptr);

	Vulkan::GPUDrawLightingConstants push_constants;
	push_constants.light_count = light_count;
	push_constants.point_light_count = point_light_count;
	vkCmdPushConstants(vk_buffer, pipeline.vk_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(Vulkan::GPUDrawLightingConstants), &push_constants);

	vkCmdDispatch(vk_buffer, (uint32_t)std::ceil(draw_extent.width / 16.0), (uint32_t)std::ceil(draw_extent.height / 16.0), 1);
}

void Vulkan::Renderer::draw_geometry(
    VkCommandBuffer vk_command_buffer,
    Vulkan::pipeline::BasePipeline pipeline) {
	// output.mark();

	// make sure every mesh that was loaded into the resource bank has GPU buffers
	// before we try to draw with it
	upload_pending_meshes();
	upload_pending_textures();

	// find first camera in scene to render with
	auto engine_cameras = engine->registry.view<Ecs::InstanceComponent, Ecs::CameraComponent>();
	auto first_camera = engine->registry.get<Ecs::CameraComponent>(engine_cameras.front());
	auto first_camera_instance = engine->registry.get<Ecs::InstanceComponent>(engine_cameras.front());

	glm::mat4 projection = glm::perspective(
	    first_camera.field_of_view,
	    (float)draw_extent.width / (float)draw_extent.height,
	    0.1f,
	    1000.0f);

	projection[1][1] *= -1;

	// output.trace("drawing objects");

	glm::mat4 camera_transform = projection * glm::inverse(first_camera_instance.transform);

	// accumulate world transforms by walking up the parent chain, memoizing each
	// entity's result so a shared ancestor is only resolved once per draw pass
	std::unordered_map<entt::entity, glm::mat4> world_transforms;
	auto world_transform = [this, &world_transforms](entt::entity entity, auto &&self) -> glm::mat4 {
		auto it = world_transforms.find(entity);
		if (it != world_transforms.end()) return it->second;

		const Ecs::InstanceComponent &instance = engine->registry.get<Ecs::InstanceComponent>(entity);
		glm::mat4 transform = instance.transform;
		if (instance.parent != entt::null && engine->registry.any_of<Ecs::InstanceComponent>(instance.parent))
			transform = self(instance.parent, self) * transform;

		world_transforms[entity] = transform;
		return transform;
	};

	// draw pass for all objects
	std::vector<MeshGeometry> &meshes = engine->resources.meshes;
	auto view = engine->registry.view<Ecs::InstanceComponent, Ecs::MeshComponent>();
	for (entt::entity entity : view) {
		const Ecs::MeshComponent &mesh = view.get<Ecs::MeshComponent>(entity);

		const Vulkan::GPUMeshBuffers &buffers = gpu_meshes[mesh.mesh_index];

		// Then, we use push-constants to upload the vertexBufferAdress to the gpu. For the matrix, we use the
		// entity's accumulated world transform (local transform composed with all parents).
		Vulkan::GPUDrawPrepassConstants push_constants;
		push_constants.object_transform = world_transform(entity, world_transform);
		push_constants.camera_transform = camera_transform;
		push_constants.vertexBuffer = buffers.vertex_buffer_address;
		push_constants.has_texture = false;

		pipeline.bindings.clear();

		if (engine->registry.any_of<Ecs::PBRMaterialComponent>(entity)) {
			const Ecs::PBRMaterialComponent &material = engine->registry.get<Ecs::PBRMaterialComponent>(entity);

			// TODO use VK_EXT_descriptor_indexing
			// You declare one large array of combined image samplers (or sampled images),
			// // populate it with all your textures, and bind it once.
			// Each object just carries a texture index, which you pass via push constant or an instance/storage buffer,
			// and index into the array in the shader (texture(textures[nonuniformEXT(idx)], uv)).

			// output.info("drawing with gpu texture %i", material.texture_index);
			Vulkan::AllocatedImage texture_view = gpu_textures[material.texture_index];

			push_constants.color = glm::vec3(1.0f, 1.0f, 1.0f);
			pipeline.bindings.push_back(Vulkan::pipeline::ImageBinding(0, &texture_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
			push_constants.has_texture = true;
		} else if (engine->registry.any_of<Ecs::BaseColorMaterialComponent>(entity)) {
			const Ecs::BaseColorMaterialComponent &material = engine->registry.get<Ecs::BaseColorMaterialComponent>(entity);

			push_constants.has_texture = false;
			push_constants.color = material.color;
			pipeline.bindings.push_back(Vulkan::pipeline::ImageBinding(0, &_errorCheckerboardImage, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
		} else {
			// no material draw state
			push_constants.has_texture = true;
			pipeline.bindings.push_back(Vulkan::pipeline::ImageBinding(0, &_errorCheckerboardImage, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
		}

		pipeline.bind(device.vk_device, vk_command_buffer, get_current_frame().frame_descriptors);

		vkCmdPushConstants(vk_command_buffer, pipeline.vk_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Vulkan::GPUDrawPrepassConstants), &push_constants);

		// We then need to do a cmdBindIndexBuffer to bind the index buffer for graphics. Sadly there is no way of
		// using device adress here, and you need to give it the VkBuffer and offsets.
		vkCmdBindIndexBuffer(vk_command_buffer, buffers.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Last, we use vkCmdDrawIndexed to draw the mesh. This is the same as the vkCmdDraw, but it uses the
		// currently bound index buffer to draw meshes.
		uint32_t index_count = (uint32_t)meshes[mesh.mesh_index].indices.size();
		vkCmdDrawIndexed(vk_command_buffer, index_count, 1, 0, 0, 0);
	}

	vkCmdEndRendering(vk_command_buffer);
}

void Vulkan::Renderer::draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view) {
	// dynamic rendering pass that loads the existing swapchain contents (the blitted
	// gradient) and draws the imgui draw data on top of it.
	VkRenderingAttachmentInfo colorAttachment = Vulkan::init::attachment_info(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = Vulkan::init::rendering_info(swapchain.extent, &colorAttachment, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	vkCmdEndRendering(cmd);
}