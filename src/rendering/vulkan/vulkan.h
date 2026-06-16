#include "rendering/vulkan/base/image/vulkan_image.h"
#include "rendering/vulkan/base/vulkan_bootstrap.h"
#include "rendering/vulkan/base/vulkan_descriptors.h"
#include "rendering/vulkan/base/vulkan_init.h"
#include "rendering/vulkan/base/vulkan_types.h"

#include <stdexcept>
#include <volk.h>

#define VK_CHECK(x)                                                                 \
	do {                                                                            \
		VkResult err = (x);                                                         \
		if (err != VK_SUCCESS) throw std::runtime_error("Vulkan call failed: " #x); \
	} while (0)
