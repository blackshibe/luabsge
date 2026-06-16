#include "rendering/vulkan/pipeline/vulkan_pipeline_shading.h"

#include <fstream>
#include <vector>
#include <volk.h>

// todo why is this here
// path is relative to the working directory (projects/<project>) the engine runs in
bool Vulkan::pipeline::shading::load_shader_module(const char *filePath, VkDevice device, VkShaderModule *outShaderModule) {
	// open the file with the cursor at the end so tellg gives the size directly
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	size_t fileSize = (size_t)file.tellg();

	// spirv expects the buffer to be uint32, so reserve a vector big enough
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	file.seekg(0);
	file.read((char *)buffer.data(), fileSize);
	file.close();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	// codeSize has to be in bytes
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}

	*outShaderModule = shaderModule;
	return true;
}
