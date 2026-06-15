#pragma once

#include "engine/queue.h"
#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {

	class GraphicsPipeline {
		DeletionQueue queue;

	  public:
		std::string name;

		VkPipeline vk_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout vk_layout = VK_NULL_HANDLE;
		VkDescriptorSetLayout vk_descriptor_layout = VK_NULL_HANDLE;

		GraphicsPipeline(std::string name, Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout_info, const char *vertex_shader_path, const char *fragment_shader_path, VkFormat color_attachment_format);

		void destroy();
	};

	class GraphicsPipelineBuilder {
		DeletionQueue queue;

	  public:
		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

		VkPipelineLayout vk_layout;
		VkFormat vk_color_format;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineColorBlendAttachmentState color_blend_info;
		VkPipelineMultisampleStateCreateInfo multisample_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		VkPipelineRenderingCreateInfo render_info;

		GraphicsPipelineBuilder() { clear(); }
		VkPipeline build_pipeline(std::string name, VkDevice device);
		void set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void set_input_topology(VkPrimitiveTopology topology);
		void set_polygon_mode(VkPolygonMode mode);
		void set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
		void set_multisampling_none();
		void disable_blending();
		void set_color_attachment_format(VkFormat format);
		void set_depth_format(VkFormat format);
		void disable_depthtest();

		void clear();
		void destroy();
	};

}
