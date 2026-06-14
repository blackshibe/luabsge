#pragma once

#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "engine/queue.h"

namespace Vulkan::pipeline {
	namespace shading {
		// Loads a compiled SPIR-V file into a VkShaderModule. 
		// Shader modules are only needed while building a pipeline
		bool load_shader_module(const char *filePath, VkDevice vk_device, VkShaderModule *outShaderModule);
	}

	class ComputePipeline {
		DeletionQueue queue;
		
	public: 
		VkPipeline vk_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout vk_layout = VK_NULL_HANDLE;

		ComputePipeline(Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout, const char* shader_path);

		void destroy();
	};

}
