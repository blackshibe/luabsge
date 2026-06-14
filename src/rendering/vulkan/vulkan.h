#include "rendering/vulkan/base/vulkan_bootstrap.h"
#include "rendering/vulkan/base/vulkan_descriptors.h"
#include "rendering/vulkan/base/vulkan_images.h"
#include "rendering/vulkan/base/vulkan_init.h"
#include "rendering/vulkan/base/vulkan_types.h"

#include <volk.h>
#include <stdexcept>

#define VK_CHECK(x)                                                                 \
	do {                                                                            \
		VkResult err = (x);                                                         \
		if (err != VK_SUCCESS) throw std::runtime_error("Vulkan call failed: " #x); \
	} while (0)
