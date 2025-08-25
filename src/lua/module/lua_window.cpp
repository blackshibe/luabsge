#include "lua_window.h"

BSGEWindow *window;

void lua_bsge_window_quit() {
	glfwSetWindowShouldClose(window->window, true);
}

glm::vec2 get_window_dimensions() {
	return glm::vec2(window->width, window->height);
}

bool get_window_focused() {
	return window->focused;
}

void lua_bsge_connect_window(BSGEWindow *_window, sol::state &lua) {
	window = _window;

	lua["Window"] = lua.create_table_with(
		"quit", &lua_bsge_window_quit,
		"get_window_dimensions", &get_window_dimensions,
		"get_focused", &get_window_focused);
}
