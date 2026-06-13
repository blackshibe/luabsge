#include "rendering/vulkan/vulkan_renderer.h"
#include "engine/engine.h"

#include <stdexcept>

VulkanRenderer::VulkanRenderer(EngineInstance &engine, GLFWwindow *glfw_window) {
	init_vulkan(engine, glfw_window);
}

VulkanRenderer::~VulkanRenderer() {
	if (device.device != VK_NULL_HANDLE) vkDeviceWaitIdle(device.device);

	Vulkan::destroy_swapchain(swapchain);
	Vulkan::destroy_device(device);

	if (surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance.instance, surface, nullptr);

	Vulkan::destroy_instance(instance);
}

void VulkanRenderer::init_vulkan(EngineInstance &engine, GLFWwindow *glfw_window) {
	Vulkan::InstanceBuilder builder;

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

	Vulkan::PhysicalDeviceSelector selector(instance, surface);
	auto physical_device_return = selector.prefer_discrete(true).select();
	if (!physical_device_return.has_value()) throw std::runtime_error("failed to select a Vulkan physical device");
	Vulkan::PhysicalDevice selected = physical_device_return.value();

	Vulkan::DeviceBuilder device_builder(selected);
	auto device_return = device_builder
		.add_pnext(&features_1_3)
		.add_pnext(&features_1_2)
		.build();
	if (!device_return.has_value()) throw std::runtime_error("failed to create Vulkan device");

	device = device_return.value();
	physical_device = selected.physical_device;

	int fb_width = 0;
	int fb_height = 0;
	glfwGetFramebufferSize(glfw_window, &fb_width, &fb_height);
	create_swapchain((uint32_t)fb_width, (uint32_t)fb_height);
}

void VulkanRenderer::create_swapchain(uint32_t width, uint32_t height) {
	Vulkan::SwapchainBuilder swapchain_builder(device, surface);

	auto swapchain_return = swapchain_builder
		.set_desired_format(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build();

	if (!swapchain_return.has_value()) throw std::runtime_error("failed to create Vulkan swapchain");
	swapchain = swapchain_return.value();
}
