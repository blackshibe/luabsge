#pragma once

#include <volk.h>

#include <span>
#include <vector>

namespace Vulkan {

	// Stores an array of VkDescriptorSetLayoutBinding (config structs) and builds them
	// into a VkDescriptorSetLayout (an actual vulkan object).
	struct DescriptorLayoutBuilder {
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		void add_binding(uint32_t binding, VkDescriptorType type);
		void clear();
		VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void *pNext = nullptr,
									VkDescriptorSetLayoutCreateFlags flags = 0);
	};

	// Descriptor allocation happens through a VkDescriptorPool: think of it as a memory
	// allocator for some specific descriptor types. Resetting a pool destroys all sets
	// allocated from it in one go (a fast path, great for per-frame descriptors).
	struct DescriptorAllocator {
		struct PoolSizeRatio {
			VkDescriptorType type;
			float ratio;
		};

		VkDescriptorPool pool;

		void init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void clear_descriptors(VkDevice device);
		void destroy_pool(VkDevice device);

		VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
	};

}
