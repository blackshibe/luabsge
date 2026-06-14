#include "rendering/vulkan/pipeline/vulkan_pipeline.h"
#include "rendering/vulkan/base/vulkan_bootstrap.h"
#include "vulkan/vulkan_core.h"
#include "vulkan_pipeline.h"

#include <fstream>
#include <vector>

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

Vulkan::pipeline::ComputePipeline::ComputePipeline(Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout_info, const char* shader_path) {
	VkShaderModule vk_draw_shader;
	if (!Vulkan::pipeline::shading::load_shader_module(shader_path, device.vk_device, &vk_draw_shader)) {
		throw std::runtime_error("failed to load shader/gradient.comp.spv (was the Shaders target built?)");
	}

	VK_CHECK(vkCreatePipelineLayout(device.vk_device, &vk_layout_info, nullptr, &vk_layout));

	// connect the shader module into a compute stage. pName is the entry point.
	VkPipelineShaderStageCreateInfo stage_info = {};
	stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_info.pNext = nullptr;
	stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage_info.module = vk_draw_shader;
	stage_info.pName = "main";

	VkComputePipelineCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.layout = vk_layout;
	create_info.stage = stage_info;

	VK_CHECK(vkCreateComputePipelines(device.vk_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &vk_pipeline));

	// destroy the layout (and the pipeline, if it gets created) at shutdown
	queue.push_function([this, device]() {
		vkDestroyPipelineLayout(device.vk_device, vk_layout, nullptr);
		vkDestroyPipeline(device.vk_device, vk_pipeline, nullptr);
	});

	// the shader module is only needed to build the pipeline; destroy it now
	vkDestroyShaderModule(device.vk_device, vk_draw_shader, nullptr);
}

void Vulkan::pipeline::ComputePipeline::destroy() {
	queue.flush();
}