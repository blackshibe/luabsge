#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vk_mem_alloc.h>
#include <volk.h>

namespace Vulkan {

	// Holds the data needed for an image: the VkImage alongside its default VkImageView,
	// the VMA allocation backing the image memory, and the image size and format.
	struct AllocatedImage {
		VkImage vk_image = VK_NULL_HANDLE;
		VkImageView vk_view = VK_NULL_HANDLE;
		VmaAllocation allocation = nullptr;
		VkExtent3D extent = {};
		VkFormat format = VK_FORMAT_UNDEFINED;
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

	// push constants for our mesh object draws. Layout matches the shader's std430
	// push_constant block: vec3 aligns to 16, so color starts at offset 144 (the int +
	// pad keep has_texture at 136 as a 4-byte value, then 4 bytes of padding to 144).
	struct GPUDrawPrepassConstants {
		glm::mat4 camera_transform;
		glm::mat4 object_transform;
		VkDeviceAddress vertexBuffer;

		int has_texture;
		int _pad;
		glm::vec3 color;
	};

	// std430-friendly directional light, uploaded as an array to the lighting pass
	struct GPUDirectionalLight {
		glm::vec4 direction;
		glm::vec4 color;
	};

	// std430-friendly point light, uploaded as an array to the lighting pass
	struct GPUPointLight {
		glm::vec4 position;
		glm::vec4 color;
	};

	struct GPUDrawLightingConstants {
		int light_count;
		int point_light_count;
	};

}
