#pragma once

#include <volk.h>

#include <deque>
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

		VkDescriptorPool pool = VK_NULL_HANDLE;

		void init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void clear_descriptors(VkDevice device);
		void destroy_pool(VkDevice device);

		VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
	};

	// Accumulates descriptor writes (image/buffer) and flushes them onto a set in one
	// vkUpdateDescriptorSets call. The deques keep the info structs alive (and their
	// addresses stable) until update_set runs.
	struct DescriptorWriter {
		std::deque<VkDescriptorImageInfo> image_infos;
		std::deque<VkDescriptorBufferInfo> buffer_infos;
		std::vector<VkWriteDescriptorSet> writes;

		void write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
		void write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

		void clear();
		void update_set(VkDevice device, VkDescriptorSet set);
	};

}
