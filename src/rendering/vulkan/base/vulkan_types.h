#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>


namespace Vulkan {

	// Holds the data needed for an image: the VkImage alongside its default VkImageView,
	// the VMA allocation backing the image memory, and the image size and format.
	struct AllocatedImage {
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VmaAllocation allocation = nullptr;
		VkExtent3D imageExtent = {};
		VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	};

}
