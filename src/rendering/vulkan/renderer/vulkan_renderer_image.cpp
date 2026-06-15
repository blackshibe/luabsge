#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/base/vulkan_init.h"
#include "rendering/vulkan/base/vulkan_types.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "engine/engine.h"
#include "resource/image/image.h"

#include "include/imgui/imgui_impl_vulkan.h"

Vulkan::AllocatedImage Vulkan::Renderer::create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	AllocatedImage image;
	image.format = format;

	VkImageCreateInfo image_info = Vulkan::init::image_create_info(format, usage, size);
	if (mipmapped) image_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo allocation_info = {};
	allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocation_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(allocator, &image_info, &allocation_info, &image.vk_image, &image.allocation, nullptr);

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	VkImageViewCreateInfo image_view_info = Vulkan::init::imageview_create_info(format, image.vk_image, aspectFlag);
	image_view_info.format = format;

	VkImageView view;
	VK_CHECK(vkCreateImageView(device.vk_device, &image_view_info, nullptr, &image.vk_view));

	return image;
}

Vulkan::AllocatedImage Vulkan::Renderer::create_image(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	size_t data_size = size.depth * size.width * size.height * 4;
	AllocatedBuffer upload_buffer = allocate_buffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(upload_buffer.info.pMappedData, data, data_size);

	AllocatedImage new_image = create_image(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	immediate_submit([&](VkCommandBuffer cmd) {
		Vulkan::util::transition_image(cmd, new_image.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = size;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, upload_buffer.buffer, new_image.vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
		                       &copyRegion);

		Vulkan::util::transition_image(cmd, new_image.vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	destroy_buffer(upload_buffer);

	return new_image;
}

void Vulkan::Renderer::destroy_image(const AllocatedImage &image) {
	vkDestroyImageView(device.vk_device, image.vk_view, nullptr);
	vmaDestroyImage(allocator, image.vk_image, image.allocation);
}

// Walks the engine resource bank and uploads to the GPU any texture that doesn't
// yet have an image. gpu_textures is kept index-aligned with resources.images.
void Vulkan::Renderer::upload_pending_textures() {
	std::vector<ImageData> &images = engine->resources.images;

	for (size_t i = gpu_textures.size(); i < images.size(); i++) {
		ImageData &image = images[i];

		VkExtent3D extent = {};
		extent.width = (uint32_t)image.width;
		extent.height = (uint32_t)image.height;
		extent.depth = 1;

		gpu_textures.push_back(create_image(image.pixels.data(), extent, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT));
	}
}