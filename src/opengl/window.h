#pragma once

#include "../glad/glad.h"
#include <GLFW/glfw3.h>
#include <lua.hpp>
#include <sol/sol.hpp>
#include <chrono>
#include <stdlib.h>

#include "freetype.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_glfw.h"
#include "../include/imgui/imgui_impl_opengl3.h"

#include "../physics/jolt.h"

class BSGEWindow {
public:
	int status = 0;
	int width = 800;
	int height = 600;
	bool focused = true;
	unsigned int default_shader;
	float program_time = 0;
	float last_frame = 0.0f;
	bool waiting_for_press_false = false;
	bool wireframe = false;
	bool should_break = false;
	int stack_warning_threshold = 100;

	GLFWwindow *window;
	sol::state *lua;
	const char *name = "Window";

	BSGEWindow();
	void init();
	void size_callback(int width, int height);
	void focus_callback(int focused);
	bool render_loop();
	void render_loop_init();
};
