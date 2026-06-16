#include "rendering/vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

Vulkan::DescriptorLayoutBuilder Vulkan::DescriptorLayoutBuilder::add_binding(uint32_t binding, VkDescriptorType type) {
	// for now we only need the binding number and descriptor type
	VkDescriptorSetLayoutBinding newbind = {};
	newbind.binding = binding;
	newbind.descriptorCount = 1;
	newbind.descriptorType = type;

	bindings.push_back(newbind);

	return *this;
}

void Vulkan::DescriptorLayoutBuilder::clear() {
	bindings.clear();
}

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
void Vulkan::DescriptorAllocator::clear_descriptors(VkDevice device) {
	vkResetDescriptorPool(device, pool, 0);
}

void Vulkan::DescriptorAllocator::destroy_pool(VkDevice device) {
	vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet Vulkan::DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
	VkDescriptorSetAllocateInfo allocation_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocation_info.pNext = nullptr;
	allocation_info.descriptorPool = pool;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &layout;

	VkDescriptorSet descriptor_set;
	VK_CHECK(vkAllocateDescriptorSets(device, &allocation_info, &descriptor_set));

	return descriptor_set;
}

// AI SLOP BEGIN
VkDescriptorPool Vulkan::DescriptorAllocatorGrowable::create_pool(VkDevice device, uint32_t set_count,
                                                                  std::span<PoolSizeRatio> pool_ratios) {
	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (PoolSizeRatio ratio : pool_ratios) {
		pool_sizes.push_back(VkDescriptorPoolSize{.type = ratio.type, .descriptorCount = uint32_t(ratio.ratio * set_count)});
	}

	VkDescriptorPoolCreateInfo pool_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pool_info.flags = 0;
	pool_info.maxSets = set_count;
	pool_info.poolSizeCount = (uint32_t)pool_sizes.size();
	pool_info.pPoolSizes = pool_sizes.data();

	VkDescriptorPool pool;
	vkCreateDescriptorPool(device, &pool_info, nullptr, &pool);
	return pool;
}

VkDescriptorPool Vulkan::DescriptorAllocatorGrowable::get_pool(VkDevice device) {
	VkDescriptorPool pool;
	if (ready_pools.size() != 0) {
		pool = ready_pools.back();
		ready_pools.pop_back();
	} else {
		pool = create_pool(device, sets_per_pool, ratios);

		sets_per_pool = sets_per_pool * 1.5;
		if (sets_per_pool > 4092) {
			sets_per_pool = 4092;
		}
	}

	return pool;
}

void Vulkan::DescriptorAllocatorGrowable::init(VkDevice device, uint32_t initial_sets, std::span<PoolSizeRatio> pool_ratios) {
	ratios.clear();
	for (PoolSizeRatio ratio : pool_ratios) {
		ratios.push_back(ratio);
	}

	VkDescriptorPool new_pool = create_pool(device, initial_sets, pool_ratios);

	sets_per_pool = initial_sets * 1.5;
	ready_pools.push_back(new_pool);
}

void Vulkan::DescriptorAllocatorGrowable::clear_pools(VkDevice device) {
	for (VkDescriptorPool pool : ready_pools) {
		vkResetDescriptorPool(device, pool, 0);
	}

	for (VkDescriptorPool pool : full_pools) {
		vkResetDescriptorPool(device, pool, 0);
		ready_pools.push_back(pool);
	}

	full_pools.clear();
}

void Vulkan::DescriptorAllocatorGrowable::destroy_pools(VkDevice device) {
	for (VkDescriptorPool pool : ready_pools) {
		vkDestroyDescriptorPool(device, pool, nullptr);
	}
	ready_pools.clear();

	for (VkDescriptorPool pool : full_pools) {
		vkDestroyDescriptorPool(device, pool, nullptr);
	}
	full_pools.clear();
}

VkDescriptorSet Vulkan::DescriptorAllocatorGrowable::allocate(VkDevice device, VkDescriptorSetLayout layout, void *pNext) {
	VkDescriptorPool pool = get_pool(device);

	VkDescriptorSetAllocateInfo allocation_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	allocation_info.pNext = pNext;
	allocation_info.descriptorPool = pool;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &layout;

	VkDescriptorSet descriptor_set;
	VkResult result = vkAllocateDescriptorSets(device, &allocation_info, &descriptor_set);

	// the pool ran out of room: retry against a fresh pool and mark the old one full
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
		full_pools.push_back(pool);

		pool = get_pool(device);
		allocation_info.descriptorPool = pool;

		VK_CHECK(vkAllocateDescriptorSets(device, &allocation_info, &descriptor_set));
	}

	ready_pools.push_back(pool);
	return descriptor_set;
}
// AI SLOP END

void Vulkan::DescriptorWriter::write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
	VkDescriptorImageInfo &info = image_infos.emplace_back(VkDescriptorImageInfo{
	    .sampler = sampler,
	    .imageView = image,
	    .imageLayout = layout});

	VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &info;

	writes.push_back(write);
}

void Vulkan::DescriptorWriter::write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
	VkDescriptorBufferInfo &info = buffer_infos.emplace_back(VkDescriptorBufferInfo{
	    .buffer = buffer,
	    .offset = offset,
	    .range = size});

	VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = &info;

	writes.push_back(write);
}

void Vulkan::DescriptorWriter::clear() {
	image_infos.clear();
	buffer_infos.clear();
	writes.clear();
}

void Vulkan::DescriptorWriter::update_set(VkDevice device, VkDescriptorSet set) {
	for (VkWriteDescriptorSet &write : writes) {
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}
