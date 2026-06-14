#pragma once

#include <volk.h>
#include "rendering/vulkan/base/vulkan_bootstrap.h"
#include "rendering/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

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
