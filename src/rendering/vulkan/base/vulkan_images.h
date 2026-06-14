#pragma once

#include <volk.h>

namespace Vulkan::util {

	// Transition an image as part of a command buffer instead of using a renderpass.
	// Transitioning an image has loads of possible options; this is the absolute
	// simplest way, using only currentLayout + newLayout. It does a pipeline barrier
	// via the synchronization-2 feature (part of Vulkan 1.3).
	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

	// vkCmdBlitImage lets us copy between images of different formats and sizes (unlike
	// vkCmdCopyImage which requires matching resolution). Source/target rectangles are
	// given and the system scales between them.
	void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

}
