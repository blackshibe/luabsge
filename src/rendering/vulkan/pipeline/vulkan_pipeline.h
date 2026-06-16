
#pragma once

#include "rendering/vulkan/base/vulkan_descriptors.h"
#include "rendering/vulkan/base/vulkan_types.h"
#include "util/output.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>

// abstract pipeline base

// TODO convert to render pass
// pipeline manages the shader config
// this also additionally manages render image lifetime and binding which is more than pipelines do

namespace Vulkan::pipeline {

	// replacement for rendergraph
	struct ImageBinding {
		uint32_t binding;
		VkDescriptorType type;
		VkImageLayout layout;
		const Vulkan::AllocatedImage *image;
		VkSampler sampler; // VK_NULL_HANDLE for storage

	  public:
		ImageBinding(
		    uint32_t binding,
		    const Vulkan::AllocatedImage *image,
		    VkSampler sampler,
		    VkImageLayout layout,
		    VkDescriptorType type) : binding(binding), type(type), layout(layout), image(image), sampler(sampler) {}
	};

	class BasePipeline {
	  public:
		std::vector<ImageBinding> bindings;

		// pipeline configuration
		VkPipeline vk_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout vk_layout = VK_NULL_HANDLE;

		// idk how this made it here
		VkDescriptorSet vk_descriptor = VK_NULL_HANDLE;
		VkDescriptorSetLayout vk_descriptor_layout = VK_NULL_HANDLE;

		BasePipeline() {};

		VkPipelineBindPoint point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		void bind(VkDevice vk_device, VkCommandBuffer vk_command_buffer, DescriptorAllocatorGrowable &allocator) {

			VkDescriptorSet image_set = allocator.allocate(vk_device, vk_descriptor_layout);
			DescriptorWriter writer;

			for (auto &binding : bindings)
				writer.write_image(binding.binding, binding.image->vk_view, binding.sampler, binding.layout, binding.type);

			writer.update_set(vk_device, image_set);
			vkCmdBindDescriptorSets(vk_command_buffer,
			                        point,
			                        vk_layout,
			                        0, 1,
			                        &image_set,
			                        0, nullptr);
		}
	};

}