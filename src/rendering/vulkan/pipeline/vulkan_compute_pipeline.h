#pragma once

#include "engine/queue.h"
#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {

	class ComputePipeline {
		DeletionQueue queue;
		std::string name;

	  public:
		VkPipeline vk_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout vk_layout = VK_NULL_HANDLE;
		VkDescriptorSet vk_descriptor = VK_NULL_HANDLE;
		VkDescriptorSetLayout vk_descriptor_layout = VK_NULL_HANDLE;

		ComputePipeline(std::string name, Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout, const char *shader_path);

		void destroy();
	};

}
