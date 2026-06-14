#pragma once

#include <GLFW/glfw3.h>
#include <lua.hpp>
#include <sol/sol.hpp>

#include <entt/entt.hpp>

#include <memory>

#include "rendering/vulkan/vulkan.h"
#include "rendering/vulkan/vulkan_renderer.h"

class EngineInstance;

class WindowInstance {
  public:
	GLFWwindow *glfw_window;
	EngineInstance &engine;

	// main functions
	virtual bool render_loop();
	virtual void render_pass();
	virtual void render_loop_init();

	// callbacks
	virtual void callback_resized(int width, int height);
	virtual void callback_focused(int focused);

	WindowInstance(EngineInstance &engine);
};

struct WindowConfiguration {
	int width;
	int height;
	const char *name;

	WindowConfiguration(int width, int height, const char *name) : width(width), height(height), name(name) {}
};

class VulkanWindowInstance : public WindowInstance {
  public:
	std::unique_ptr<Vulkan::Renderer> vulkan_renderer;

	VulkanWindowInstance(EngineInstance &engine);

	bool render_loop();
	void render_loop_init();
};