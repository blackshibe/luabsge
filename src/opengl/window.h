#pragma once

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <lua.hpp>
#include <sol/sol.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class BSGEWindow {
public:
	int status = 0;
	int width = 800;
	int height = 600;
	unsigned int default_shader;
	float program_time = 0;

	GLFWwindow *window;
	sol::state *lua;
	const char *name = "Window";

	BSGEWindow();
	void init();
	void size_callback(int width, int height);
	void render_loop();
};
