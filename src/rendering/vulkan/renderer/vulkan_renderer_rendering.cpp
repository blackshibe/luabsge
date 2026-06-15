#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/pipeline/vulkan_graphics_pipeline.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "ecs/instance.h"
#include "ecs/scene_camera.h"
#include "ecs/scene_mesh.h"
#include "engine/engine.h"
#include "util/output.h"
#include "vulkan/vulkan_core.h"

#include <cmath>
#include <vector>

static Output output;

void Vulkan::Renderer::draw() {

	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	// (in nanoseconds). Fences have to be reset between uses.
	VK_CHECK(vkWaitForFences(device.vk_device, 1, &get_current_frame()._renderFence, true, 1000000000));

	// the fence is signalled, so the gpu finished this frame's work: safe to flush any
	// per-frame resources now.
	get_current_frame()._deletionQueue.flush();
	get_current_frame().frame_descriptors.clear_descriptors(device.vk_device);

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

	// todo change type to depth
	Vulkan::util::transition_image(vk_command_buffer, pipelines.get()->image_depth.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	Vulkan::Renderer::draw_geometry_pipeline(vk_command_buffer, pipelines.get()->image_depth.vk_view, pipelines.get()->prepass_depth);

	Vulkan::util::transition_image(vk_command_buffer, pipelines.get()->image_albedo.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	Vulkan::Renderer::draw_geometry_pipeline(vk_command_buffer, pipelines.get()->image_albedo.vk_view, pipelines.get()->prepass_albedo);

	// LIGHTING PIPELINE ---- LIGHTING PIPELINE ----------------------------------------------------------------------
	// the compute shader samples the albedo prepass (binding 0) and writes the result into
	// draw_image (binding 1, a storage image). Move albedo to a sampleable layout and the
	// draw image to GENERAL so the shader can write it.
	Vulkan::util::transition_image(vk_command_buffer, pipelines.get()->image_albedo.vk_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	Vulkan::util::transition_image(vk_command_buffer, draw_image.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	VkDescriptorSet imageSet = get_current_frame().frame_descriptors.allocate(device.vk_device, pipelines.get()->pass_lighting.vk_descriptor_layout);
	Vulkan::DescriptorWriter writer;
	writer.write_image(0, pipelines.get()->image_albedo.vk_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_image(1, pipelines.get()->image_depth.vk_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_image(2, draw_image.vk_view, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	writer.update_set(device.vk_device, imageSet);

	Vulkan::Renderer::draw_pipeline(vk_command_buffer, pipelines.get()->pass_lighting, imageSet);

	// PRESENT IMAGE --------------------------------------------------------------------------------------------------
	// transition the draw image (written by the lighting compute pass) and the swapchain
	// image into their transfer layouts, then copy the draw image into the swapchain image.
	Vulkan::util::transition_image(vk_command_buffer, draw_image.vk_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::util::transition_image(vk_command_buffer, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Vulkan::util::copy_image_to_image(vk_command_buffer, draw_image.vk_image, swapchain.images[swapchainImageIndex], draw_extent, swapchain.extent);

	// draw imgui directly onto the swapchain image. We need it in color-attachment
	// layout for dynamic rendering, then transition to present.
	Vulkan::util::transition_image(vk_command_buffer, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	draw_imgui(vk_command_buffer, swapchain.image_views[swapchainImageIndex]);

	// make the swapchain image into presentable mode. PRESENT_SRC_KHR is the only
	// layout the swapchain allows for presenting to screen.
	Vulkan::util::transition_image(vk_command_buffer, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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

void Vulkan::Renderer::draw_pipeline(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline, VkDescriptorSet descriptor_set) {
	// bind the compute pipeline and the descriptor set holding its input/output images,
	// then dispatch. The shader uses a 16x16 workgroup, so we divide the draw resolution
	// by 16 and round up.
	vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_pipeline);
	vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_layout, 0, 1, &descriptor_set, 0, nullptr);
	vkCmdDispatch(vk_buffer, (uint32_t)std::ceil(draw_extent.width / 16.0), (uint32_t)std::ceil(draw_extent.height / 16.0), 1);
}

void Vulkan::Renderer::draw_geometry_pipeline(VkCommandBuffer vk_command_buffer, VkImageView target_view, Vulkan::pipeline::GraphicsPipeline pipeline) {
	// begin a render pass connected to this pass's target image (e.g. the depth or
	// albedo prepass image), which the caller has already transitioned to
	// COLOR_ATTACHMENT_OPTIMAL.
	VkRenderingAttachmentInfo colorAttachment = Vulkan::init::attachment_info(target_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = Vulkan::init::rendering_info(draw_extent, &colorAttachment, nullptr);
	vkCmdBeginRendering(vk_command_buffer, &renderInfo);

	// we do a CmdBindPipeline, but instead of using BIND_POINT_COMPUTE, we now use
	// VK_PIPELINE_BIND_POINT_GRAPHICS.
	vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);

	// set dynamic viewport and scissor. This is required before as we left them
	// undefined when creating the pipeline (we were using dynamic pipeline state).
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = draw_extent.width;
	viewport.height = draw_extent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(vk_command_buffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = draw_extent.width;
	scissor.extent.height = draw_extent.height;

	vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);

	// make sure every mesh that was loaded into the resource bank has GPU buffers
	// before we try to draw with it
	upload_pending_meshes();
	upload_pending_textures();

	// We bind another pipeline, this time the rectangle mesh one.
	vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);

	// bind a texture: use the first imported texture if there is one, otherwise fall
	// back to the magenta/black checkerboard
	VkImageView texture_view = gpu_textures.empty() ? _errorCheckerboardImage.vk_view : gpu_textures[0].vk_view;

	VkDescriptorSet imageSet = get_current_frame().frame_descriptors.allocate(device.vk_device, pipeline.vk_descriptor_layout);
	{
		Vulkan::DescriptorWriter writer;
		writer.write_image(0, texture_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		writer.update_set(device.vk_device, imageSet);
	}

	vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_layout, 0, 1, &imageSet, 0, nullptr);

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

	glm::mat4 camera_transform = projection * glm::inverse(first_camera_instance.transform);

	// Draw every entity that has a mesh: each MeshComponent stores an index into the
	// resource bank, which is kept index-aligned with gpu_meshes, so we can look up
	// the GPU buffers directly and draw the geometry the same way the tutorial draws
	// its single rectangle.
	std::vector<MeshGeometry> &meshes = engine->resources.meshes;
	auto view = engine->registry.view<Ecs::InstanceComponent, Ecs::MeshComponent>();
	for (entt::entity entity : view) {
		const Ecs::InstanceComponent &instance = view.get<Ecs::InstanceComponent>(entity);
		const Ecs::MeshComponent &mesh = view.get<Ecs::MeshComponent>(entity);

		if (mesh.mesh_index < 0 || (size_t)mesh.mesh_index >= gpu_meshes.size()) continue;

		const Vulkan::GPUMeshBuffers &buffers = gpu_meshes[mesh.mesh_index];

		// Then, we use push-constants to upload the vertexBufferAdress to the gpu. For the matrix, we use the
		// entity's transform (it defaults to identity until we implement mesh transformations / a camera).
		Vulkan::GPUDrawPrepassConstants push_constants;
		push_constants.object_transform = instance.transform;
		push_constants.camera_transform = camera_transform;
		push_constants.vertexBuffer = buffers.vertex_buffer_address;

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