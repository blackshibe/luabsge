#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vk_mem_alloc.h>
#include <volk.h>

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

	struct AllocatedBuffer {
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo info;
	};

	// holds the resources needed for a mesh
	struct GPUMeshBuffers {

		AllocatedBuffer index_buffer;
		AllocatedBuffer vertex_buffer;
		VkDeviceAddress vertex_buffer_address;
	};

	// push constants for our mesh object draws
	struct GPUDrawPushConstants {
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};

}
