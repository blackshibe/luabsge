#pragma once

#include <volk.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace Vulkan {

	struct Instance {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
		uint32_t api_version = VK_API_VERSION_1_0;
		bool validation_enabled = false;
	};

	class InstanceBuilder {
	public:
		InstanceBuilder &set_app_name(const char *name);
		InstanceBuilder &set_engine_name(const char *name);
		InstanceBuilder &require_api_version(uint32_t version);
		InstanceBuilder &request_validation_layers(bool enable = true);
		InstanceBuilder &use_default_debug_messenger();
		InstanceBuilder &enable_extension(const char *name);
		InstanceBuilder &enable_extensions(const char *const *names, uint32_t count);
		InstanceBuilder &enable_layer(const char *name);

		std::optional<Instance> build() const;

	private:
		const char *app_name = "luabsge";
		const char *engine_name = "luabsge";
		uint32_t api_version = VK_API_VERSION_1_3;
		bool validation = false;
		bool debug_messenger = false;
		std::vector<const char *> extensions;
		std::vector<const char *> layers;
	};

	struct PhysicalDevice {
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceFeatures features{};
		VkPhysicalDeviceMemoryProperties memory_properties{};
		uint32_t graphics_queue_family = VK_QUEUE_FAMILY_IGNORED;
		uint32_t present_queue_family = VK_QUEUE_FAMILY_IGNORED;
		std::vector<const char *> extensions;
	};

	class PhysicalDeviceSelector {
	public:
		PhysicalDeviceSelector(const Instance &instance, VkSurfaceKHR surface);

		PhysicalDeviceSelector &set_surface(VkSurfaceKHR surface);
		PhysicalDeviceSelector &add_required_extension(const char *name);
		PhysicalDeviceSelector &prefer_discrete(bool enable = true);
		PhysicalDeviceSelector &set_required_features(VkPhysicalDeviceFeatures features);

		std::optional<PhysicalDevice> select() const;

	private:
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		std::vector<const char *> required_extensions;
		bool prefer_discrete_gpu = true;
		VkPhysicalDeviceFeatures required_features{};
	};

	struct Device {
		VkDevice device = VK_NULL_HANDLE;
		PhysicalDevice physical_device;
		VkQueue graphics_queue = VK_NULL_HANDLE;
		VkQueue present_queue = VK_NULL_HANDLE;
	};

	class DeviceBuilder {
	public:
		DeviceBuilder(const PhysicalDevice &physical_device);

		DeviceBuilder &add_pnext(void *structure);

		std::optional<Device> build() const;

	private:
		PhysicalDevice physical_device;
		std::vector<void *> pnext_chain;
	};

	struct Swapchain {
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkFormat image_format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
		VkExtent2D extent{};
		uint32_t image_count = 0;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;

		bool get_images();
		bool create_image_views();
		void destroy_image_views();
	};

	class SwapchainBuilder {
	public:
		SwapchainBuilder(const Device &device, VkSurfaceKHR surface);

		SwapchainBuilder &set_desired_extent(uint32_t width, uint32_t height);
		SwapchainBuilder &set_desired_format(VkSurfaceFormatKHR format);
		SwapchainBuilder &set_desired_present_mode(VkPresentModeKHR mode);
		SwapchainBuilder &set_desired_image_count(uint32_t count);
		SwapchainBuilder &set_image_usage_flags(VkImageUsageFlags usage);
		SwapchainBuilder &set_old_swapchain(VkSwapchainKHR old_swapchain);

		std::optional<Swapchain> build() const;

	private:
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		uint32_t graphics_queue_family = VK_QUEUE_FAMILY_IGNORED;
		uint32_t present_queue_family = VK_QUEUE_FAMILY_IGNORED;

		uint32_t desired_width = 0;
		uint32_t desired_height = 0;
		uint32_t desired_image_count = 0;
		VkSurfaceFormatKHR desired_format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
		VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
		VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
	};

	void destroy_swapchain(const Swapchain &swapchain);
	void destroy_device(const Device &device);
	void destroy_instance(const Instance &instance);

}
