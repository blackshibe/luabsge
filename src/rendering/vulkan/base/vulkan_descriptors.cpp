#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

void Vulkan::DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type) {
	// for now we only need the binding number and descriptor type
	VkDescriptorSetLayoutBinding newbind = {};
	newbind.binding = binding;
	newbind.descriptorCount = 1;
	newbind.descriptorType = type;

	bindings.push_back(newbind);
}

void Vulkan::DescriptorLayoutBuilder::clear() { bindings.clear(); }

VkDescriptorSetLayout Vulkan::DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void *pNext,
															 VkDescriptorSetLayoutCreateFlags flags) {
	// we don't support per-binding stage flags; force the same stages for the whole set
	for (auto &b : bindings) {
		b.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	info.pNext = pNext;
	info.pBindings = bindings.data();
	info.bindingCount = (uint32_t)bindings.size();
	info.flags = flags;

	VkDescriptorSetLayout set;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

	return set;
}

void Vulkan::DescriptorAllocator::init_pool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(VkDescriptorPoolSize{.type = ratio.type, .descriptorCount = uint32_t(ratio.ratio * maxSets)});
	}

	VkDescriptorPoolCreateInfo pool_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pool_info.flags = 0;
	pool_info.maxSets = maxSets;
	pool_info.poolSizeCount = (uint32_t)poolSizes.size();
	pool_info.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
}

// not a delete: resets the pool, destroying all descriptors but keeping the pool itself
void Vulkan::DescriptorAllocator::clear_descriptors(VkDevice device) { vkResetDescriptorPool(device, pool, 0); }

void Vulkan::DescriptorAllocator::destroy_pool(VkDevice device) { vkDestroyDescriptorPool(device, pool, nullptr); }

VkDescriptorSet Vulkan::DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
	VkDescriptorSetAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet ds;
	VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &ds));

	return ds;
}
