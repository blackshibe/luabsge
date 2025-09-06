#include "lua_window.h"

BSGEWindow *window;

// Track VSync state since GLFW doesn't provide a getter
static bool vsync_enabled = false; // Start as disabled (matches current window.cpp behavior)

void lua_bsge_window_quit() {
	glfwSetWindowShouldClose(window->window, true);
}

glm::vec2 get_window_dimensions() {
	return glm::vec2(window->width, window->height);
}

bool get_window_focused() {
	return window->focused;
}

void set_vsync(bool enabled) {
	glfwSwapInterval(enabled ? 1 : 0);
	vsync_enabled = enabled;
}

bool get_vsync() {
	return vsync_enabled;
}

void set_window_size(int width, int height) {
	glfwSetWindowSize(window->window, width, height);
}

void set_fullscreen(bool enabled) {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	
	if (enabled) {
		glfwSetWindowMonitor(window->window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	} else {
		glfwSetWindowMonitor(window->window, nullptr, 100, 100, 800, 600, 0);
	}
}

void maximize_window() {
	glfwMaximizeWindow(window->window);
}

void lua_bsge_connect_window(BSGEWindow *_window, sol::state &lua) {
	window = _window;

	lua["Window"] = lua.create_table_with(
		"quit", &lua_bsge_window_quit,
		"get_window_dimensions", &get_window_dimensions,
		"get_focused", &get_window_focused,
		"set_vsync", &set_vsync,
		"get_vsync", &get_vsync,
		"set_window_size", &set_window_size,
		"set_fullscreen", &set_fullscreen,
		"maximize", &maximize_window);
}
