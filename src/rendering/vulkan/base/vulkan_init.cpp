#include "rendering/vulkan/vulkan.h"

VkFenceCreateInfo Vulkan::init::fence_create_info(VkFenceCreateFlags flags) {
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	return info;
}

VkSemaphoreCreateInfo Vulkan::init::semaphore_create_info(VkSemaphoreCreateFlags flags) {
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = flags;
	return info;
}

VkCommandBufferBeginInfo Vulkan::init::command_buffer_begin_info(VkCommandBufferUsageFlags flags) {
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.pInheritanceInfo = nullptr;
	info.flags = flags;
	return info;
}

VkImageSubresourceRange Vulkan::init::image_subresource_range(VkImageAspectFlags aspectMask) {
	VkImageSubresourceRange subImage = {};
	subImage.aspectMask = aspectMask;
	subImage.baseMipLevel = 0;
	// transition all mipmap levels and layers
	subImage.levelCount = VK_REMAINING_MIP_LEVELS;
	subImage.baseArrayLayer = 0;
	subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;
	return subImage;
}

VkSemaphoreSubmitInfo Vulkan::init::semaphore_submit_info(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
	VkSemaphoreSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.semaphore = semaphore;
	submitInfo.stageMask = stageMask;
	// deviceIndex is for multi-gpu, value is for timeline semaphores; neither used here
	submitInfo.deviceIndex = 0;
	submitInfo.value = 1;
	return submitInfo;
}

VkCommandBufferSubmitInfo Vulkan::init::command_buffer_submit_info(VkCommandBuffer cmd) {
	VkCommandBufferSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBuffer = cmd;
	info.deviceMask = 0;
	return info;
}

VkSubmitInfo2 Vulkan::init::submit_info(VkCommandBufferSubmitInfo *cmd, VkSemaphoreSubmitInfo *signalSemaphoreInfo,
										VkSemaphoreSubmitInfo *waitSemaphoreInfo) {
	VkSubmitInfo2 info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.pNext = nullptr;

	info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
	info.pWaitSemaphoreInfos = waitSemaphoreInfo;

	info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
	info.pSignalSemaphoreInfos = signalSemaphoreInfo;

	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = cmd;

	return info;
}

VkImageCreateInfo Vulkan::init::image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;
	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;

	// MSAA: default to 1 sample per pixel
	info.samples = VK_SAMPLE_COUNT_1_BIT;

	// optimal tiling: the image is stored in the best gpu format
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usageFlags;

	return info;
}

VkImageViewCreateInfo Vulkan::init::imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
}

VkRenderingAttachmentInfo Vulkan::init::attachment_info(VkImageView view, VkClearValue *clear, VkImageLayout layout) {
	VkRenderingAttachmentInfo colorAttachment = {};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.pNext = nullptr;
	colorAttachment.imageView = view;
	colorAttachment.imageLayout = layout;
	colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	if (clear) colorAttachment.clearValue = *clear;
	return colorAttachment;
}

VkRenderingInfo Vulkan::init::rendering_info(VkExtent2D renderExtent, VkRenderingAttachmentInfo *colorAttachment,
											 VkRenderingAttachmentInfo *depthAttachment) {
	VkRenderingInfo renderInfo = {};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.pNext = nullptr;
	renderInfo.renderArea = VkRect2D{VkOffset2D{0, 0}, renderExtent};
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = colorAttachment;
	renderInfo.pDepthAttachment = depthAttachment;
	renderInfo.pStencilAttachment = nullptr;
	return renderInfo;
}
