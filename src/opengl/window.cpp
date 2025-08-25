#include "window.h"

#include "../include/colors.h"
#include "../lua/class/camera.h"
#include "../lua/class/gizmo.h"
#include "../lua/class/input.h"
#include "../lua/module/lua_rendering.h"

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
	
	// Set up framebuffer size callback and initialize viewport
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		BSGEWindow* win = static_cast<BSGEWindow*>(glfwGetWindowUserPointer(window));
		win->size_callback(width, height);
	});
	
	// Set up window focus callback
	glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused) {
		BSGEWindow* win = static_cast<BSGEWindow*>(glfwGetWindowUserPointer(window));
		win->focus_callback(focused);
	});
	
	// Get the actual framebuffer size and initialize viewport
	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
	size_callback(framebuffer_width, framebuffer_height);
	printf("[window.cpp] initialized viewport with framebuffer callback: %i, %i\n", 
		   framebuffer_width, framebuffer_height);
	
	// Initialize input system
	// TODO move everything to lua_window
	set_input_window(window);
}

// todo: dynamic resizing that works with textlabels
void BSGEWindow::size_callback(int width, int height) {
	printf("[window.cpp] resized window to %i, %i\n", width, height);

	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);

	freetype_resize_window(width, height);
}

void BSGEWindow::focus_callback(int focused) {
	this->focused = (focused == GLFW_TRUE);
}

void BSGEWindow::render_loop() {

	// MODEL SHADER CODE
	unsigned int default_shader;
	bool success = bsge_compile_shader(&default_shader, "shader/mesh/vertex_default.glsl", "shader/mesh/frag_default.glsl");
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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	bool should_break = false;
	while (!glfwWindowShouldClose(window) && !should_break) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			if (!waiting_for_press_false) {
				wireframe = !wireframe;
				if (!wireframe) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				} else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
			}
			waiting_for_press_false = true;
		} else {
			waiting_for_press_false = false;
		}

		// frame start
		float current_frame = glfwGetTime();
		float delta_time = current_frame - last_frame;
		
		// update input system
		update_mouse_input();

		// clear the screen
		glClearColor(0.1, 0.1, 0.1, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// get camera projection and coordinates from the Lua context using sol2
		sol::table world = (*lua)["World"];
		sol::table rendering = world["rendering"];
		sol::optional<BSGECameraMetadata*> camera_opt = rendering["camera"];
		
		if (!camera_opt) {
			printf("%s", ANSI_RED);
			printf("[window.cpp] no camera defined!\n");
			printf("[window.cpp] as there is currently no fallback, the window will shut down.\n");
			printf("%s", ANSI_NC);

			glfwSetWindowShouldClose(window, true);
			return;
		}
		
		BSGECameraMetadata* camera = camera_opt.value();
		glm::mat4 camera_projection = camera_get_projection_matrix(*camera);

		glUseProgram(default_shader);
		glUniformMatrix4fv(glGetUniformLocation(default_shader, "camera_transform"), 1, GL_FALSE, glm::value_ptr(camera->matrix));
		glUniformMatrix4fv(glGetUniformLocation(default_shader, "projection"), 1, GL_FALSE, glm::value_ptr(camera_projection));

		// imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// gizmo
		lua_bsge_gizmo_begin_frame(camera_projection, camera->matrix);

		// lua
		bsge_call_lua_render(lua, delta_time);

		lua_bsge_gizmo_end_frame();
		ImGui::Render();

		// Check lua stack size for potential leaks
		lua_State *L = lua->lua_state();
		int stack_size = lua_gettop(L);
		if (stack_warning_threshold < stack_size) {
			printf("Stack size over limit! Is there a leak? size: %i\n", stack_size);
			should_break = true;
		}

		// frame end
		float calc_time = glfwGetTime() - current_frame;
		last_frame = current_frame;

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();

		const GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			printf("[main.cpp] exit: OpenGL error: %i\n", err);
		}
	}
}
