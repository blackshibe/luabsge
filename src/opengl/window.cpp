#include "window.h"
#include "shader.h"
#include "freetype.h"

#include "../lua/module/lua_rendering.h"
#include "../include/colors.h"
#include "../lua/class/camera.h"

#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_glfw.h"
#include "../include/imgui/imgui_impl_opengl3.h"

#include <stdlib.h>
#include <chrono>

BSGEWindow::BSGEWindow() {
	this->window = glfwCreateWindow(width, height, name, NULL, NULL);
}

void BSGEWindow::init() {
	if (this->window == NULL) {
		printf("[window.cpp] exit: failed to create GLFW window\n");
		glfwTerminate();
		status = -1;
		return;
	}

	glfwMakeContextCurrent(this->window);
	glfwSetWindowUserPointer(this->window, this);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("[window.cpp] exit: failed to initialize GLAD\n");
		glfwTerminate();
		status = -1;
		return;
	}

	glEnable(GL_DEPTH_TEST);
}


void BSGEWindow::size_callback(int width, int height) {
	printf("[window.cpp] resized window to %i, %i\n", width, height);

	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);

	// todo: dynamic resizing that works with textlabels
	if (width > height) {
		this->height = width;
		glViewport(0, (height - width) / 2, width, width);
	}
	else {
		this->width = height;
		glViewport((width - height) / 2, 0, height, height);
	}
}

void BSGEWindow::render_loop() {

	// MODEL SHADER CODE
	unsigned int default_shader;
	bool success = bsge_compile_shader(&default_shader, "shader/vertex_default.glsl", "shader/frag_default.glsl");
	if (!success) {
		printf("[main.cpp] exit: couldn't compile default shaders\n");
		status = -1;
		return;
	};

	this->default_shader = default_shader;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// probably disables vsync
	// glfwSwapInterval(0);

	// camera projection
	float last_frame = glfwGetTime();
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	bool waiting_for_press_false = false;
	bool wireframe = false;
	int stack_warning_threshold = 100;

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			if (!waiting_for_press_false) {
				wireframe = !wireframe;
				if (!wireframe) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}

			}
			waiting_for_press_false = true;
		}
		else {
			waiting_for_press_false = false;
		}

		// frame start
		float current_frame = glfwGetTime();
		float delta_time = current_frame - last_frame;

		// clear the screen
		glClearColor(0.1, 0.1, 0.1, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// get camera projection and coordinates from the Lua context
		luaL_getmetatable(L, "Rendering");
		lua_getfield(L, -1, "camera");
		if (lua_isnil(L, -1)) {
			printf("%s", ANSI_RED);

			printf("[window.cpp] no camera defined!\n");
			printf("[window.cpp] as there is currently no fallback, the window will shut down.\n");

			printf("%s", ANSI_NC);

			glfwSetWindowShouldClose(window, true);
		} else {
			BSGECameraMetadata* camera = (BSGECameraMetadata*)lua_touserdata(L, -1);
			glm::mat4 proj = glm::perspective(glm::radians(camera->fov), (float)width / (float)height, camera->near_clip, camera->far_clip);

			glUseProgram(default_shader);
			glUniformMatrix4fv(glGetUniformLocation(default_shader, "camera_transform"), 1, GL_FALSE, glm::value_ptr(camera->position));
			glUniformMatrix4fv(glGetUniformLocation(default_shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

			auto t1 = std::chrono::high_resolution_clock::now();

			// run the lua shit
			bsge_call_lua_render(L, delta_time);

			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();

			{
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();

				ImGui::NewFrame();
				ImGui::Begin("LuaBSGE Engine Info"); 
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text("Lua context runtime %.2f ms last frame", duration);
				
				ImGui::End();
			}

			ImGui::Render();
		}

		// clear the stack from the camera check
		lua_remove(L, -1);
		lua_remove(L, -1);
		
		int stack_size = lua_gettop(L);
		if (stack_warning_threshold < stack_size) printf("Stack size over limit! Is there a leak? size: %i\n", stack_size);

		// frame end
		float calc_time = glfwGetTime() - current_frame;
		last_frame = current_frame;

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		const GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			printf("[main.cpp] exit: OpenGL error: %i\n", err);
		}
	}
}
