#pragma once

#include "rendering/vulkan/vulkan_bootstrap.h"
#include <GLFW/glfw3.h>

class EngineInstance;

class Renderer {};

class VulkanRenderer : Renderer {
    Vulkan::Instance instance;
    Vulkan::Device device;
    Vulkan::Swapchain swapchain;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    void init_vulkan(EngineInstance &engine, GLFWwindow *glfw_window);
    void create_swapchain(uint32_t width, uint32_t height);

public:
    VulkanRenderer(EngineInstance &engine, GLFWwindow *glfw_window);
    ~VulkanRenderer();
};
