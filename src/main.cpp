
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include <lua.hpp>

#include "math.h"
#include "unistd.h"

#include "lua/lua.h"
#include "lua/luax.h"
#include "opengl/freetype.h"
#include "opengl/window.h"

// https://learnopengl.com/Getting-started/Transformations
// https://csl.name/post/lua-and-cpp/
// https://www.lua.org/pil/28.1.html

void window_resize(GLFWwindow *window, int width, int height) {
	((BSGEWindow *)glfwGetWindowUserPointer(window))->size_callback(width, height);
	freetype_resize_window(width, height);
}

void err(int error_code, const char *description) {
	printf("[main.cpp] glfw error callback called because '%s'\n", description);
}

int main(int argc, char *argv[]) {
	printf("[main.cpp] running %s\n", LUA_VERSION);

	glfwInit();
	glfwSetErrorCallback(err);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	sol::state lua;
	BSGEWindow window = BSGEWindow();
	window.init();
	window.lua = &lua;

	if (window.status != 0) {
		return EXIT_FAILURE;
	}

	freetype_init(lua);
	if (bsge_lua_init_state(&window, lua) == -1) {
		return EXIT_FAILURE;
	}

	if (luax_run_script(lua, "entry.lua")) {
		glfwSetFramebufferSizeCallback(window.window, window_resize);

		if (window.status == -1) {
			return EXIT_FAILURE;
		}

		window_resize(window.window, window.width, window.height);
		window.render_loop();
	} else {
		printf("[main.cpp] entry.lua failed to run. game closed.\n");
	}

	glfwTerminate();
	freetype_quit();

	return 0;
}
