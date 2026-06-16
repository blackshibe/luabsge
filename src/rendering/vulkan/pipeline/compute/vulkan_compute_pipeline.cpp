#include "vulkan_compute_pipeline.h"

#include "rendering/vulkan/base/vulkan_bootstrap.h"
#include "rendering/vulkan/pipeline/vulkan_pipeline_shading.h"
#include "util/output.h"
#include "vulkan/vulkan_core.h"

static Output output(LogDomain::Vulkan);

Vulkan::pipeline::ComputePipeline::ComputePipeline(std::string name, Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout_info, const char *shader_path) {
	output.mark();

	this->name = name;

	VkShaderModule vk_draw_shader;
	if (!Vulkan::pipeline::shading::load_shader_module(shader_path, device.vk_device, &vk_draw_shader)) {
		throw std::runtime_error("failed to load shader/gradient.comp.spv (was the Shaders target built?)");
	}

	// create layout
	VK_CHECK(vkCreatePipelineLayout(device.vk_device, &vk_layout_info, nullptr, &vk_layout));

	vk_descriptor_layout = *vk_layout_info.pSetLayouts;

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

	output.info("creating pipeline %s", name.data());
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