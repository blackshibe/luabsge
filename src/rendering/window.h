#pragma once

#include <GLFW/glfw3.h>
#include <lua.hpp>
#include <sol/sol.hpp>

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	const char* name;

	WindowConfiguration(int width, int height, const char *name): width(width), height(height), name(name) {}
};

class VulkanWindowInstance : public WindowInstance {
public:
	VulkanWindowInstance(EngineInstance &engine);

};