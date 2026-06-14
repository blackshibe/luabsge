#pragma once

#include "engine/queue.h"
#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {

	class ComputePipeline {
		DeletionQueue queue;

	  public:
		VkPipeline vk_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout vk_layout = VK_NULL_HANDLE;

		ComputePipeline(Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout, const char *shader_path);

		void destroy();
	};

}
