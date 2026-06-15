#include "rendering/vulkan/pipeline/vulkan_graphics_pipeline.h"

#include "rendering/vulkan/base/vulkan_bootstrap.h"
#include "rendering/vulkan/pipeline/vulkan_pipeline_shading.h"
#include "util/output.h"
#include "vulkan/vulkan_core.h"
#include <stdexcept>

static Output output;

Vulkan::pipeline::GraphicsPipeline::GraphicsPipeline(std::string name, Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout_info, const char *vertex_shader_path, const char *fragment_shader_path, VkFormat color_attachment_format) {
	output.mark();

	this->name = name;

	// We will start by loading the 2 shaders into VkShaderModules, like we did with the
	// compute shader, but this time more shaders.
	VkShaderModule vertex_shader;
	if (!Vulkan::pipeline::shading::load_shader_module(vertex_shader_path, device.vk_device, &vertex_shader)) {
		throw std::runtime_error("failed to load triangle vertex shader (was the Shaders target built?)");
	}

	VkShaderModule fragment_shader;
	if (!Vulkan::pipeline::shading::load_shader_module(fragment_shader_path, device.vk_device, &fragment_shader)) {
		throw std::runtime_error("failed to load triangle fragment shader (was the Shaders target built?)");
	}

	// We also create the pipeline layout. Unlike with the compute shader before, this
	// time we have no push constants and no descriptor bindings on here, so its really
	// just a completely empty layout.
	VK_CHECK(vkCreatePipelineLayout(device.vk_device, &vk_layout_info, nullptr, &vk_layout));

	// Now we create the pipeline, using the Pipeline Builder created before.
	GraphicsPipelineBuilder builder;
	// use the triangle layout we created
	builder.vk_layout = vk_layout;
	// connecting the vertex and pixel shaders to the pipeline
	builder.set_shaders(vertex_shader, fragment_shader);
	// it will draw triangles
	builder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	// filled triangles
	builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	// no backface culling
	builder.set_cull_mode(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_CLOCKWISE);
	// no multisampling
	builder.set_multisampling_none();
	// no blending
	builder.disable_blending();
	// no depth testing
	builder.disable_depthtest();

	// connect the image format we will draw into, from draw image
	builder.set_color_attachment_format(color_attachment_format);
	builder.set_depth_format(VK_FORMAT_UNDEFINED);

	// finally build the pipeline
	vk_pipeline = builder.build_pipeline(name, device.vk_device);

	vk_descriptor_layout = *vk_layout_info.pSetLayouts;

	// clean structures: the shader modules are only needed to build the pipeline
	vkDestroyShaderModule(device.vk_device, fragment_shader, nullptr);
	vkDestroyShaderModule(device.vk_device, vertex_shader, nullptr);

	// destroy the layout and the pipeline at shutdown
	queue.push_function([this, device]() {
		vkDestroyPipelineLayout(device.vk_device, vk_layout, nullptr);
		vkDestroyPipeline(device.vk_device, vk_pipeline, nullptr);
	});
}

VkPipeline Vulkan::pipeline::GraphicsPipelineBuilder::build_pipeline(std::string name, VkDevice device) {
	output.mark();

	// make viewport state from our stored viewport and scissor.
	// at the moment we wont support multiple viewports or scissors
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// setup dummy color blending. We arent using transparent objects yet
	// the blending is just "no blend", but we do write to the color attachment
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &color_blend_info;

	// completely clear VertexInputStateCreateInfo, as we have no need for it
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	// build the actual pipeline
	// we now use all of the info structs we have been writing into into this one
	// to create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

	// connect the renderInfo to the pNext extension mechanism
	pipelineInfo.pNext = &render_info;
	pipelineInfo.stageCount = (uint32_t)shader_stages.size();
	pipelineInfo.pStages = shader_stages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &input_assembly_info;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterization_info;
	pipelineInfo.pMultisampleState = &multisample_info;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDepthStencilState = &depth_stencil_info;
	pipelineInfo.layout = vk_layout;

	VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

	dynamicInfo.pDynamicStates = &state[0];
	dynamicInfo.dynamicStateCount = 2;

	pipelineInfo.pDynamicState = &dynamicInfo;

	// its easy to error out on create graphics pipeline, so we handle it a bit
	// better than the common VK_CHECK case
	VkPipeline newPipeline;
	output.info("building pipeline %s", name.data());
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline");
	} else {
		return newPipeline;
	}
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader) {
	shader_stages.clear();

	shader_stages.push_back(
	    Vulkan::init::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));

	shader_stages.push_back(
	    Vulkan::init::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_input_topology(VkPrimitiveTopology topology) {
	input_assembly_info.topology = topology;
	// we are not going to use primitive restart on the entire tutorial so leave
	// it on false
	input_assembly_info.primitiveRestartEnable = VK_FALSE;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
	rasterization_info.polygonMode = mode;
	rasterization_info.lineWidth = 1.f;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace) {
	rasterization_info.cullMode = cullMode;
	rasterization_info.frontFace = frontFace;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_multisampling_none() {
	multisample_info.sampleShadingEnable = VK_FALSE;
	// multisampling defaulted to no multisampling (1 sample per pixel)
	multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_info.minSampleShading = 1.0f;
	multisample_info.pSampleMask = nullptr;
	// no alpha to coverage either
	multisample_info.alphaToCoverageEnable = VK_FALSE;
	multisample_info.alphaToOneEnable = VK_FALSE;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::disable_blending() {
	// default write mask
	color_blend_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	// no blending
	color_blend_info.blendEnable = VK_FALSE;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_color_attachment_format(VkFormat format) {
	vk_color_format = format;
	// connect the format to the renderInfo  structure
	render_info.colorAttachmentCount = 1;
	render_info.pColorAttachmentFormats = &vk_color_format;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::set_depth_format(VkFormat format) {
	render_info.depthAttachmentFormat = format;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::disable_depthtest() {
	depth_stencil_info.depthTestEnable = VK_FALSE;
	depth_stencil_info.depthWriteEnable = VK_FALSE;
	depth_stencil_info.depthCompareOp = VK_COMPARE_OP_NEVER;
	depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_info.stencilTestEnable = VK_FALSE;
	depth_stencil_info.front = {};
	depth_stencil_info.back = {};
	depth_stencil_info.minDepthBounds = 0.f;
	depth_stencil_info.maxDepthBounds = 1.f;
}

void Vulkan::pipeline::GraphicsPipelineBuilder::clear() {
	input_assembly_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	rasterization_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	color_blend_info = {};
	multisample_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	depth_stencil_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	render_info = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
	shader_stages.clear();
}

void Vulkan::pipeline::GraphicsPipelineBuilder::destroy() {
	queue.flush();
}

void Vulkan::pipeline::GraphicsPipeline::destroy() {
	queue.flush();
}
