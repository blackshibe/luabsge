#include "rendering/vulkan/vk_bootstrap.h"

#include "include/colors.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <set>

namespace Vulkan {

	static void log_error(const char *fmt, VkResult result) {
		printf("%s[vk_bootstrap] %s (VkResult %d)%s\n", ANSI_RED, fmt, (int)result, ANSI_NC);
	}

	static void log_error(const char *fmt) {
		printf("%s[vk_bootstrap] %s%s\n", ANSI_RED, fmt, ANSI_NC);
	}

	static bool has_layer(const std::vector<VkLayerProperties> &available, const char *name) {
		for (const VkLayerProperties &layer : available)
			if (strcmp(layer.layerName, name) == 0)
				return true;
		return false;
	}

	static bool has_extension(const std::vector<VkExtensionProperties> &available, const char *name) {
		for (const VkExtensionProperties &extension : available)
			if (strcmp(extension.extensionName, name) == 0)
				return true;
		return false;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL default_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data) {

		const char *color = ANSI_BLUE;
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			color = ANSI_BOLD_RED;
		else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			color = ANSI_BOLD_YELLOW;

		printf("%s[vulkan] %s%s\n", color, data->pMessage, ANSI_NC);
		return VK_FALSE;
	}

	static VkDebugUtilsMessengerCreateInfoEXT default_messenger_info() {
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = default_debug_callback;
		return info;
	}

	InstanceBuilder &InstanceBuilder::set_app_name(const char *name) {
		app_name = name;
		return *this;
	}

	InstanceBuilder &InstanceBuilder::set_engine_name(const char *name) {
		engine_name = name;
		return *this;
	}

	InstanceBuilder &InstanceBuilder::require_api_version(uint32_t version) {
		api_version = version;
		return *this;
	}

	InstanceBuilder &InstanceBuilder::request_validation_layers(bool enable) {
		validation = enable;
		return *this;
	}

	InstanceBuilder &InstanceBuilder::use_default_debug_messenger() {
		debug_messenger = true;
		return *this;
	}

	InstanceBuilder &InstanceBuilder::enable_extension(const char *name) {
		extensions.push_back(name);
		return *this;
	}

	InstanceBuilder &InstanceBuilder::enable_extensions(const char *const *names, uint32_t count) {
		for (uint32_t i = 0; i < count; i++)
			extensions.push_back(names[i]);
		return *this;
	}

	InstanceBuilder &InstanceBuilder::enable_layer(const char *name) {
		layers.push_back(name);
		return *this;
	}

	std::optional<Instance> InstanceBuilder::build() const {
		if (volkInitialize() != VK_SUCCESS) {
			log_error("volkInitialize failed; is a Vulkan driver installed?");
			return std::nullopt;
		}

		std::vector<const char *> enabled_extensions = extensions;
		std::vector<const char *> enabled_layers = layers;

		bool enable_validation = validation;
		bool enable_debug_messenger = debug_messenger;

		if (enable_validation) {
			uint32_t layer_count = 0;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			std::vector<VkLayerProperties> available_layers(layer_count);
			vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

			if (has_layer(available_layers, "VK_LAYER_KHRONOS_validation")) {
				enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
			} else {
				printf("%s[vk_bootstrap] validation requested but VK_LAYER_KHRONOS_validation not present; skipping%s\n", ANSI_BOLD_YELLOW, ANSI_NC);
				enable_validation = false;
				enable_debug_messenger = false;
			}
		}

		if (enable_debug_messenger)
			enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = app_name;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = engine_name;
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = api_version;

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = (uint32_t)enabled_extensions.size();
		create_info.ppEnabledExtensionNames = enabled_extensions.data();
		create_info.enabledLayerCount = (uint32_t)enabled_layers.size();
		create_info.ppEnabledLayerNames = enabled_layers.data();

		VkDebugUtilsMessengerCreateInfoEXT messenger_info = default_messenger_info();
		if (enable_debug_messenger)
			create_info.pNext = &messenger_info;

		Instance result;
		VkResult status = vkCreateInstance(&create_info, nullptr, &result.instance);
		if (status != VK_SUCCESS) {
			log_error("vkCreateInstance failed", status);
			return std::nullopt;
		}

		volkLoadInstance(result.instance);

		if (enable_debug_messenger) {
			status = vkCreateDebugUtilsMessengerEXT(result.instance, &messenger_info, nullptr, &result.debug_messenger);
			if (status != VK_SUCCESS)
				log_error("vkCreateDebugUtilsMessengerEXT failed", status);
		}

		result.api_version = api_version;
		result.validation_enabled = enable_validation;
		return result;
	}

	PhysicalDeviceSelector::PhysicalDeviceSelector(const Instance &instance, VkSurfaceKHR surface)
		: instance(instance.instance), surface(surface) {
		required_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	PhysicalDeviceSelector &PhysicalDeviceSelector::set_surface(VkSurfaceKHR surface) {
		this->surface = surface;
		return *this;
	}

	PhysicalDeviceSelector &PhysicalDeviceSelector::add_required_extension(const char *name) {
		required_extensions.push_back(name);
		return *this;
	}

	PhysicalDeviceSelector &PhysicalDeviceSelector::prefer_discrete(bool enable) {
		prefer_discrete_gpu = enable;
		return *this;
	}

	PhysicalDeviceSelector &PhysicalDeviceSelector::set_required_features(VkPhysicalDeviceFeatures features) {
		required_features = features;
		return *this;
	}

	static bool find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &graphics, uint32_t &present) {
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
		std::vector<VkQueueFamilyProperties> families(count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

		graphics = VK_QUEUE_FAMILY_IGNORED;
		present = VK_QUEUE_FAMILY_IGNORED;

		for (uint32_t i = 0; i < count; i++) {
			if (graphics == VK_QUEUE_FAMILY_IGNORED && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				graphics = i;

			VkBool32 supports_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present);
			if (present == VK_QUEUE_FAMILY_IGNORED && supports_present)
				present = i;

			if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				if (supports_present) {
					graphics = i;
					present = i;
				}
			}
		}

		return graphics != VK_QUEUE_FAMILY_IGNORED && present != VK_QUEUE_FAMILY_IGNORED;
	}

	static bool supports_required_extensions(VkPhysicalDevice device, const std::vector<const char *> &required) {
		uint32_t count = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> available(count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

		for (const char *name : required)
			if (!has_extension(available, name))
				return false;
		return true;
	}

	static bool supports_swapchain(VkPhysicalDevice device, VkSurfaceKHR surface) {
		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

		return format_count > 0 && present_mode_count > 0;
	}

	std::optional<PhysicalDevice> PhysicalDeviceSelector::select() const {
		if (surface == VK_NULL_HANDLE) {
			log_error("PhysicalDeviceSelector requires a surface");
			return std::nullopt;
		}

		uint32_t count = 0;
		vkEnumeratePhysicalDevices(instance, &count, nullptr);
		if (count == 0) {
			log_error("no Vulkan physical devices found");
			return std::nullopt;
		}

		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(instance, &count, devices.data());

		PhysicalDevice best{};
		int best_score = -1;

		for (VkPhysicalDevice device : devices) {
			if (!supports_required_extensions(device, required_extensions))
				continue;
			if (!supports_swapchain(device, surface))
				continue;

			uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
			uint32_t present = VK_QUEUE_FAMILY_IGNORED;
			if (!find_queue_families(device, surface, graphics, present))
				continue;

			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(device, &properties);

			int score = 0;
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += prefer_discrete_gpu ? 1000 : 100;
			score += (int)(properties.limits.maxImageDimension2D);

			if (score > best_score) {
				best_score = score;
				best = PhysicalDevice{};
				best.physical_device = device;
				best.properties = properties;
				vkGetPhysicalDeviceFeatures(device, &best.features);
				vkGetPhysicalDeviceMemoryProperties(device, &best.memory_properties);
				best.graphics_queue_family = graphics;
				best.present_queue_family = present;
				best.extensions = required_extensions;
			}
		}

		if (best_score < 0) {
			log_error("no suitable physical device (needs graphics+present queue, swapchain, required extensions)");
			return std::nullopt;
		}

		printf("[vk_bootstrap] selected GPU: %s\n", best.properties.deviceName);
		return best;
	}

	DeviceBuilder::DeviceBuilder(const PhysicalDevice &physical_device) : physical_device(physical_device) {}

	DeviceBuilder &DeviceBuilder::add_pnext(void *structure) {
		pnext_chain.push_back(structure);
		return *this;
	}

	std::optional<Device> DeviceBuilder::build() const {
		std::set<uint32_t> unique_families = {
			physical_device.graphics_queue_family,
			physical_device.present_queue_family,
		};

		float priority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queue_infos;
		for (uint32_t family : unique_families) {
			VkDeviceQueueCreateInfo queue_info{};
			queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_info.queueFamilyIndex = family;
			queue_info.queueCount = 1;
			queue_info.pQueuePriorities = &priority;
			queue_infos.push_back(queue_info);
		}

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = (uint32_t)queue_infos.size();
		create_info.pQueueCreateInfos = queue_infos.data();
		create_info.enabledExtensionCount = (uint32_t)physical_device.extensions.size();
		create_info.ppEnabledExtensionNames = physical_device.extensions.data();
		create_info.pEnabledFeatures = &physical_device.features;

		void **next = (void **)&create_info.pNext;
		for (void *structure : pnext_chain) {
			*next = structure;
			next = (void **)&((VkBaseOutStructure *)structure)->pNext;
		}

		Device result;
		result.physical_device = physical_device;
		VkResult status = vkCreateDevice(physical_device.physical_device, &create_info, nullptr, &result.device);
		if (status != VK_SUCCESS) {
			log_error("vkCreateDevice failed", status);
			return std::nullopt;
		}

		volkLoadDevice(result.device);

		vkGetDeviceQueue(result.device, physical_device.graphics_queue_family, 0, &result.graphics_queue);
		vkGetDeviceQueue(result.device, physical_device.present_queue_family, 0, &result.present_queue);
		return result;
	}

	SwapchainBuilder::SwapchainBuilder(const Device &device, VkSurfaceKHR surface)
		: physical_device(device.physical_device.physical_device),
		  device(device.device),
		  surface(surface),
		  graphics_queue_family(device.physical_device.graphics_queue_family),
		  present_queue_family(device.physical_device.present_queue_family) {}

	SwapchainBuilder &SwapchainBuilder::set_desired_extent(uint32_t width, uint32_t height) {
		desired_width = width;
		desired_height = height;
		return *this;
	}

	SwapchainBuilder &SwapchainBuilder::set_desired_format(VkSurfaceFormatKHR format) {
		desired_format = format;
		return *this;
	}

	SwapchainBuilder &SwapchainBuilder::set_desired_present_mode(VkPresentModeKHR mode) {
		desired_present_mode = mode;
		return *this;
	}

	SwapchainBuilder &SwapchainBuilder::set_desired_image_count(uint32_t count) {
		desired_image_count = count;
		return *this;
	}

	SwapchainBuilder &SwapchainBuilder::set_image_usage_flags(VkImageUsageFlags usage) {
		image_usage = usage;
		return *this;
	}

	SwapchainBuilder &SwapchainBuilder::set_old_swapchain(VkSwapchainKHR old) {
		old_swapchain = old;
		return *this;
	}

	static VkSurfaceFormatKHR choose_format(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR desired) {
		uint32_t count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

		for (const VkSurfaceFormatKHR &format : formats)
			if (format.format == desired.format && format.colorSpace == desired.colorSpace)
				return format;
		return formats[0];
	}

	static VkPresentModeKHR choose_present_mode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR desired) {
		uint32_t count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
		std::vector<VkPresentModeKHR> modes(count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

		for (VkPresentModeKHR mode : modes)
			if (mode == desired)
				return mode;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	std::optional<Swapchain> SwapchainBuilder::build() const {
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

		VkSurfaceFormatKHR surface_format = choose_format(physical_device, surface, desired_format);
		VkPresentModeKHR present_mode = choose_present_mode(physical_device, surface, desired_present_mode);

		VkExtent2D extent;
		if (capabilities.currentExtent.width != UINT32_MAX) {
			extent = capabilities.currentExtent;
		} else {
			extent.width = std::clamp(desired_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(desired_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		uint32_t image_count = desired_image_count != 0 ? desired_image_count : capabilities.minImageCount + 1;
		if (image_count < capabilities.minImageCount)
			image_count = capabilities.minImageCount;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
			image_count = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = image_usage;
		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = old_swapchain;

		uint32_t families[] = {graphics_queue_family, present_queue_family};
		if (graphics_queue_family != present_queue_family) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = families;
		} else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		Swapchain result;
		result.device = device;
		result.image_format = surface_format.format;
		result.color_space = surface_format.colorSpace;
		result.present_mode = present_mode;
		result.extent = extent;

		VkResult status = vkCreateSwapchainKHR(device, &create_info, nullptr, &result.swapchain);
		if (status != VK_SUCCESS) {
			log_error("vkCreateSwapchainKHR failed", status);
			return std::nullopt;
		}

		if (!result.get_images() || !result.create_image_views()) {
			destroy_swapchain(result);
			return std::nullopt;
		}

		return result;
	}

	bool Swapchain::get_images() {
		uint32_t count = 0;
		vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
		images.resize(count);
		vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());
		image_count = count;
		return count > 0;
	}

	bool Swapchain::create_image_views() {
		image_views.resize(images.size());
		for (size_t i = 0; i < images.size(); i++) {
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = images[i];
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = image_format;
			info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.layerCount = 1;

			VkResult status = vkCreateImageView(device, &info, nullptr, &image_views[i]);
			if (status != VK_SUCCESS) {
				log_error("vkCreateImageView failed", status);
				return false;
			}
		}
		return true;
	}

	void Swapchain::destroy_image_views() {
		for (VkImageView view : image_views)
			if (view != VK_NULL_HANDLE)
				vkDestroyImageView(device, view, nullptr);
		image_views.clear();
	}

	void destroy_swapchain(const Swapchain &swapchain) {
		Swapchain mutable_copy = swapchain;
		mutable_copy.destroy_image_views();
		if (swapchain.swapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(swapchain.device, swapchain.swapchain, nullptr);
	}

	void destroy_device(const Device &device) {
		if (device.device != VK_NULL_HANDLE)
			vkDestroyDevice(device.device, nullptr);
	}

	void destroy_instance(const Instance &instance) {
		if (instance.debug_messenger != VK_NULL_HANDLE)
			vkDestroyDebugUtilsMessengerEXT(instance.instance, instance.debug_messenger, nullptr);
		if (instance.instance != VK_NULL_HANDLE)
			vkDestroyInstance(instance.instance, nullptr);
	}

}
