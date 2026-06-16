#pragma once

#include "engine/queue.h"
#include "rendering/vulkan/pipeline/vulkan_pipeline.h"
#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {

	class GraphicsPipeline : public BasePipeline {
		DeletionQueue queue;

	  public:
		std::string name;

		// todo should this be here?
		void transition_color_image(VkCommandBuffer vk_command_buffer, VkImageLayout next_layout);
		AllocatedImage color_image;
		VkImageLayout color_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		void transition_depth_image(VkCommandBuffer vk_command_buffer, VkImageLayout next_layout);
		AllocatedImage depth_image;
		VkImageLayout depth_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

		GraphicsPipeline(std::string name, Vulkan::Device device, VkPipelineLayoutCreateInfo vk_layout_info, const char *vertex_shader_path, const char *fragment_shader_path, AllocatedImage color_image, AllocatedImage depth_image);

		void bind_for_render(VkCommandBuffer vk_command_buffer, VkExtent2D draw_extent);

		void destroy();

		VkPipelineBindPoint point = VK_PIPELINE_BIND_POINT_GRAPHICS;
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
