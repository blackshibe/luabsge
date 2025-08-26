#include "main.h"

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
#if USE_EMSCRIPTEN
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

#if !USE_EMSCRIPTEN
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

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
		window.render_loop_init();
	} else {
		printf("[main.cpp] entry.lua failed to run. game closed.\n");
	}

	glfwTerminate();
	freetype_quit();

	return 0;
}
