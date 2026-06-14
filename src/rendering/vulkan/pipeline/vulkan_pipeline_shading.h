#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {
	namespace shading {
		// Loads a compiled SPIR-V file into a VkShaderModule.
		// Shader modules are only needed while building a pipeline
		bool load_shader_module(const char *filePath, VkDevice vk_device, VkShaderModule *outShaderModule);
	}
}