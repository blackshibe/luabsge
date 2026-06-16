#pragma once

#include "engine/queue.h"
#include "rendering/vulkan/pipeline/vulkan_pipeline.h"
#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {

	class ComputePipeline : public BasePipeline {
		DeletionQueue queue;
		std::string name;

	  public:
		ComputePipeline(std::string name, Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout, const char *shader_path);

		void destroy();

		VkPipelineBindPoint point = VK_PIPELINE_BIND_POINT_COMPUTE;
	};

}
