#include "rendering/vulkan/vulkan_renderer.h"

#include "engine/engine.h"

#include <stdexcept>

Vulkan::Renderer::Renderer(EngineInstance &engine, GLFWwindow *glfw_window) {
	init_vulkan(engine, glfw_window);
}

Vulkan::Renderer::~Renderer() {
	if (device.vk_device != VK_NULL_HANDLE) vkDeviceWaitIdle(device.vk_device);

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		// free any per-frame resources before destroying the frame data
		_frames[i]._deletionQueue.flush();

		if (_frames[i]._commandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(device.vk_device, _frames[i]._commandPool, nullptr);

		if (_frames[i]._renderFence != VK_NULL_HANDLE)
			vkDestroyFence(device.vk_device, _frames[i]._renderFence, nullptr);
		if (_frames[i]._swapchainSemaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(device.vk_device, _frames[i]._swapchainSemaphore, nullptr);
	}

	for (VkSemaphore semaphore : render_semaphores) {
		if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device.vk_device, semaphore, nullptr);
	}

	// flush the global deletion queue (draw image, descriptors, pipelines, allocator).
	// This must run while the device is still alive but before it is destroyed.
	lifetime_deletion_queue.flush();

	Vulkan::Bootstrap::destroy_swapchain(swapchain);
	Vulkan::Bootstrap::destroy_device(device);

	if (surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance.instance, surface, nullptr);

	Vulkan::Bootstrap::destroy_instance(instance);
}

void Vulkan::Renderer::init_vulkan(EngineInstance &engine, GLFWwindow *glfw_window) {
	Vulkan::Bootstrap::InstanceBuilder builder;

    // glfw requires some vulkan extensions
    uint32_t glfw_ext_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
		 
    auto instance_return = builder.set_app_name(Engine::VERSION)
        .request_validation_layers(true)
        .use_default_debug_messenger()
        .require_api_version(VK_API_VERSION_1_3)
        .enable_extensions(glfw_extensions, glfw_ext_count)
        .build();

    if (!instance_return.has_value()) throw std::runtime_error("failed to create Vulkan instance");
    instance = instance_return.value();

    if (glfwCreateWindowSurface(instance.instance, glfw_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

	VkPhysicalDeviceVulkan13Features features_1_3{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features_1_3.dynamicRendering = true;
	features_1_3.synchronization2 = true;

	VkPhysicalDeviceVulkan12Features features_1_2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features_1_2.bufferDeviceAddress = true;
	features_1_2.descriptorIndexing = true;

	Vulkan::Bootstrap::PhysicalDeviceSelector selector(instance, surface);
	auto physical_device_return = selector.prefer_discrete(true).select();
	if (!physical_device_return.has_value()) throw std::runtime_error("failed to select a Vulkan physical device");
	Vulkan::PhysicalDevice selected = physical_device_return.value();

	Vulkan::Bootstrap::DeviceBuilder device_builder(selected);
	auto device_return = device_builder
		.add_pnext(&features_1_3)
		.add_pnext(&features_1_2)
		.build();
	if (!device_return.has_value()) throw std::runtime_error("failed to create Vulkan device");

	device = device_return.value();
	physical_device = selected.vk_device;
	queue_family = selected.graphics_queue_family;

	// initialize the memory allocator. We give VMA the volk-loaded
	// vkGetInstanceProcAddr/vkGetDeviceProcAddr so it can resolve the rest of the
	// functions it needs (we build with VK_NO_PROTOTYPES). The BUFFER_DEVICE_ADDRESS
	// flag lets us use GPU pointers later.
	VmaVulkanFunctions vulkan_functions = {};
	vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.physicalDevice = physical_device;
	allocator_info.device = device.vk_device;
	allocator_info.instance = instance.instance;
	allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocator_info.pVulkanFunctions = &vulkan_functions;
	vmaCreateAllocator(&allocator_info, &allocator);

	lifetime_deletion_queue.push_function([this]() { vmaDestroyAllocator(allocator); });

	int fb_width = 0;
	int fb_height = 0;
	glfwGetFramebufferSize(glfw_window, &fb_width, &fb_height);
	create_swapchain((uint32_t)fb_width, (uint32_t)fb_height);

	init_commands();
	init_sync_structures();
	init_descriptors();
	init_pipelines();
	init_imgui(glfw_window);
}

void Vulkan::Renderer::create_swapchain(uint32_t width, uint32_t height) {
	Vulkan::Bootstrap::SwapchainBuilder swapchain_builder(device, surface);

	auto swapchain_return = swapchain_builder
		.set_desired_format(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build();

	if (!swapchain_return.has_value()) throw std::runtime_error("failed to create Vulkan swapchain");
	swapchain = swapchain_return.value();

	// draw image size matches the window. We hardcode RGBA 16-bit float for the extra
	// precision that helps with lighting calculations and avoids banding.
	VkExtent3D drawImageExtent = {width, height, 1};
	draw_image.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	draw_image.imageExtent = drawImageExtent;

	// TransferSRC/DST to copy from and into it, Storage so a compute shader can write
	// to it, ColorAttachment so graphics pipelines can draw into it.
	VkImageUsageFlags drawImageUsages = 0;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = Vulkan::init::image_create_info(draw_image.imageFormat, drawImageUsages, drawImageExtent);

	// allocate the draw image from gpu local memory (VRAM) for fastest access
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &draw_image.image, &draw_image.allocation, nullptr);

	// build the default image view we pair with the image to access it
	VkImageViewCreateInfo rview_info = Vulkan::init::imageview_create_info(draw_image.imageFormat, draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);
	VK_CHECK(vkCreateImageView(device.vk_device, &rview_info, nullptr, &draw_image.imageView));

	lifetime_deletion_queue.push_function([this]() {
		vkDestroyImageView(device.vk_device, draw_image.imageView, nullptr);
		vmaDestroyImage(allocator, draw_image.image, draw_image.allocation);
	});
}


