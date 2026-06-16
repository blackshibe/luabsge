#include "rendering/vulkan/base/vulkan_descriptors.h"
#include "vulkan/vulkan_core.h"

namespace Vulkan::pipeline {

	class AbstractRenderPass {
	  public:
		// pipeline configuration
		VkPipeline vk_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout vk_layout = VK_NULL_HANDLE;

		// idk how this made it here
		VkDescriptorSet vk_descriptor = VK_NULL_HANDLE;
		VkDescriptorSetLayout vk_descriptor_layout = VK_NULL_HANDLE;

		// descriptor for image sampler layout (?)
		// basically configures uniforms (?)
		Vulkan::DescriptorWriter descriptor_writer;

		AbstractRenderPass();
	};

}