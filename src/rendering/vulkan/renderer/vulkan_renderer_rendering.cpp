#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "ecs/instance.h"
#include "ecs/scene_camera.h"
#include "ecs/scene_mesh.h"
#include "engine/engine.h"

#include <cmath>
#include <vector>

void Vulkan::Renderer::draw() {
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	// (in nanoseconds). Fences have to be reset between uses.
	VK_CHECK(vkWaitForFences(device.vk_device, 1, &get_current_frame()._renderFence, true, 1000000000));

	// the fence is signalled, so the gpu finished this frame's work: safe to flush any
	// per-frame resources now.
	get_current_frame()._deletionQueue.flush();
	get_current_frame()._frameDescriptors.clear_descriptors(device.vk_device);

	VK_CHECK(vkResetFences(device.vk_device, 1, &get_current_frame()._renderFence));

	// request image from the swapchain. We send the _swapchainSemaphore so we can
	// sync our render commands to the swapchain image being ready.
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device.vk_device, swapchain.swapchain, 1000000000,
	                               get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex));

	// naming it cmd for shorter writing. Vulkan handles are just 64-bit pointers,
	// so copying them around is fine.
	VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	// now that the commands finished executing (we waited on the fence), we can
	// safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	// the draw region matches our off-swapchain draw image
	draw_extent.width = draw_image.extent.width;
	draw_extent.height = draw_image.extent.height;

	// we will use this command buffer exactly once, so tell vulkan that for a
	// possible small speedup in command encoding.
	VkCommandBufferBeginInfo cmdBeginInfo = Vulkan::init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// transition our main draw image into general layout so we can write into it. We
	// will overwrite it all so we dont care about the older layout.
	Vulkan::util::transition_image(cmd, draw_image.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// record the actual draw commands into the draw image
	// boy i sure hope this shit isn't null!
	// draw_pipeline(cmd, *gradient_pipeline.get());

	// the compute shader we run for the background needed to draw into GENERAL image
	// layout, but when doing geometry rendering, we need to use COLOR_ATTACHMENT_OPTIMAL.
	// It is possible to draw into GENERAL layout with graphics pipelines, but its lower
	// performance and the validation layers will complain.
	Vulkan::util::transition_image(cmd, draw_image.vk_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	draw_geometry(cmd);

	// transition the draw image and the swapchain image into their transfer layouts,
	// then blit (copy) the draw image into the swapchain image.
	Vulkan::util::transition_image(cmd, draw_image.vk_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Vulkan::util::copy_image_to_image(cmd, draw_image.vk_image, swapchain.images[swapchainImageIndex], draw_extent, swapchain.extent);

	// draw imgui directly onto the swapchain image. We need it in color-attachment
	// layout for dynamic rendering, then transition to present.
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	draw_imgui(cmd, swapchain.image_views[swapchainImageIndex]);

	// make the swapchain image into presentable mode. PRESENT_SRC_KHR is the only
	// layout the swapchain allows for presenting to screen.
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	// prepare the submission to the queue. We wait on the _swapchainSemaphore, as it
	// is signaled when the swapchain image is ready, and we signal the _renderSemaphore
	// to indicate that rendering has finished.
	VkCommandBufferSubmitInfo cmdinfo = Vulkan::init::command_buffer_submit_info(cmd);
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

void Vulkan::Renderer::draw_pipeline(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline) {
	// bind the gradient compute pipeline and the descriptor set holding the draw image,
	// then dispatch. The shader uses a 16x16 workgroup, so we divide the draw resolution
	// by 16 and round up.
	vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_pipeline);
	vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_layout, 0, 1, &draw_image_descriptors, 0, nullptr);
	vkCmdDispatch(vk_buffer, (uint32_t)std::ceil(draw_extent.width / 16.0), (uint32_t)std::ceil(draw_extent.height / 16.0), 1);
}

void Vulkan::Renderer::draw_geometry(VkCommandBuffer cmd) {
	// begin a render pass connected to our draw image. This is the same we were doing
	// for imgui, but this time we are pointing it into our draw image instead of the
	// swapchain image.
	VkRenderingAttachmentInfo colorAttachment = Vulkan::init::attachment_info(draw_image.vk_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = Vulkan::init::rendering_info(draw_extent, &colorAttachment, nullptr);
	vkCmdBeginRendering(cmd, &renderInfo);

	// we do a CmdBindPipeline, but instead of using BIND_POINT_COMPUTE, we now use
	// VK_PIPELINE_BIND_POINT_GRAPHICS.
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, triangle_pipeline->vk_pipeline);

	// set dynamic viewport and scissor. This is required before as we left them
	// undefined when creating the pipeline (we were using dynamic pipeline state).
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = draw_extent.width;
	viewport.height = draw_extent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = draw_extent.width;
	scissor.extent.height = draw_extent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// launch a draw command to draw 3 vertices
	vkCmdDraw(cmd, 3, 1, 0, 0);

	// make sure every mesh that was loaded into the resource bank has GPU buffers
	// before we try to draw with it
	upload_pending_meshes();
	upload_pending_textures();

	// We bind another pipeline, this time the rectangle mesh one.
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipeline->vk_pipeline);

	// bind a texture: use the first imported texture if there is one, otherwise fall
	// back to the magenta/black checkerboard
	VkImageView texture_view = gpu_textures.empty() ? _errorCheckerboardImage.vk_view : gpu_textures[0].vk_view;

	VkDescriptorSet imageSet = get_current_frame()._frameDescriptors.allocate(device.vk_device, _singleImageDescriptorLayout);
	{
		Vulkan::DescriptorWriter writer;
		writer.write_image(0, texture_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		writer.update_set(device.vk_device, imageSet);
	}

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipeline->vk_layout, 0, 1, &imageSet, 0, nullptr);

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
		Vulkan::GPUDrawPushConstants push_constants;
		push_constants.object_transform = instance.transform;
		push_constants.camera_transform = camera_transform;
		push_constants.vertexBuffer = buffers.vertex_buffer_address;

		vkCmdPushConstants(cmd, mesh_pipeline->vk_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Vulkan::GPUDrawPushConstants), &push_constants);

		// We then need to do a cmdBindIndexBuffer to bind the index buffer for graphics. Sadly there is no way of
		// using device adress here, and you need to give it the VkBuffer and offsets.
		vkCmdBindIndexBuffer(cmd, buffers.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Last, we use vkCmdDrawIndexed to draw the mesh. This is the same as the vkCmdDraw, but it uses the
		// currently bound index buffer to draw meshes.
		uint32_t index_count = (uint32_t)meshes[mesh.mesh_index].indices.size();
		vkCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);
	}

	vkCmdEndRendering(cmd);
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