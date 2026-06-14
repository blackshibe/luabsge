#pragma once

#include <volk.h>

namespace Vulkan::init {

	// Both of these structures are pretty simple and need almost no options other
	// than to give them some flags. We want the fence to start signalled so we can
	// wait on it on the first frame without deadlocking.
	VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
	VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);

	// When a command buffer is started, we need to give it an info struct with some
	// properties. We will not be using inheritance info so we keep it nullptr, but
	// we do need the flags (e.g. VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT).
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

	// Lets us target a part of the image with a barrier. Most useful for array or
	// mipmapped images; we default it to cover all mip levels and layers. AspectMask
	// is COLOR for color images and DEPTH for depth images.
	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);

	// VkSubmitInfo2 (synchronization-2) needs a VkSemaphoreSubmitInfo per semaphore
	// and a VkCommandBufferSubmitInfo per command buffer it enqueues.
	VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd);
	VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signalSemaphoreInfo,
							  VkSemaphoreSubmitInfo *waitSemaphoreInfo);

	// Image tiling is hardcoded to OPTIMAL: the gpu may shuffle the data however it
	// sees fit. Samples default to 1 (no MSAA). The view is the thin wrapper needed
	// to actually access the image.
	VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	// dynamic rendering: a color (or depth) attachment for vkCmdBeginRendering. With a
	// null clear value the attachment loads existing contents instead of clearing.
	VkRenderingAttachmentInfo attachment_info(VkImageView view, VkClearValue *clear, VkImageLayout layout);
	VkRenderingInfo rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachment,
								   VkRenderingAttachmentInfo *depthAttachment);

}
