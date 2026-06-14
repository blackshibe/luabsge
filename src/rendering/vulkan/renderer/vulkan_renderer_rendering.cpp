#include "rendering/vulkan/vulkan_renderer.h"
#include "include/imgui/imgui_impl_vulkan.h"

#include <cmath>

void Vulkan::Renderer::draw() {
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	// (in nanoseconds). Fences have to be reset between uses.
	VK_CHECK(vkWaitForFences(device.vk_device, 1, &get_current_frame()._renderFence, true, 1000000000));

	// the fence is signalled, so the gpu finished this frame's work: safe to flush any
	// per-frame resources now.
	get_current_frame()._deletionQueue.flush();

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
	draw_extent.width = draw_image.imageExtent.width;
	draw_extent.height = draw_image.imageExtent.height;

	// we will use this command buffer exactly once, so tell vulkan that for a
	// possible small speedup in command encoding.
	VkCommandBufferBeginInfo cmdBeginInfo = Vulkan::init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// transition our main draw image into general layout so we can write into it. We
	// will overwrite it all so we dont care about the older layout.
	Vulkan::util::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// record the actual draw commands into the draw image
	// boy i sure hope this shit isn't null!
	draw_pipeline(cmd, *gradient_pipeline.get());

	// transition the draw image and the swapchain image into their transfer layouts,
	// then blit (copy) the draw image into the swapchain image.
	Vulkan::util::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Vulkan::util::copy_image_to_image(cmd, draw_image.image, swapchain.images[swapchainImageIndex], draw_extent, swapchain.extent);

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

void Vulkan::Renderer::draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view) {
	// dynamic rendering pass that loads the existing swapchain contents (the blitted
	// gradient) and draws the imgui draw data on top of it.
	VkRenderingAttachmentInfo colorAttachment = Vulkan::init::attachment_info(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = Vulkan::init::rendering_info(swapchain.extent, &colorAttachment, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	vkCmdEndRendering(cmd);
}