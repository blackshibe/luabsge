#include "window.h"

#include "shader.h"
#include "freetype.h"
#include "../lua/module/rendering.h"

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

	// camera projection
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

	float last_frame = glfwGetTime();
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		float current_frame = glfwGetTime();
		float delta_time = current_frame - last_frame;

		// if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		// 	if (!waiting_for_press_false) {
		// 		wireframe = !wireframe;
		// 		if (!wireframe) {
		// 			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		// 			printf("reloading shaders\n");
		// 			glDeleteProgram(default_shader);

		// 			bool success = bsge_compile_shader(&default_shader, "default_shader/vertex_default.glsl", "default_shader/frag_default.glsl");
		// 			if (!success) {
		// 				printf("[main.cpp] exit: couldn't compile default shaders\n");
		// 				return EXIT_FAILURE;
		// 			};

		// 		} else {
		// 			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		// 		}



		// 	}
		// 	waiting_for_press_false = true;
		// } else {
		// 	waiting_for_press_false = false;
		// }

		glClearColor(0.1, 0.1, 0.1, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 2. use our default_shader program when we want to render an object
		glUseProgram(default_shader);

		// float timeValue = glfwGetTime();
		// float value = abs(sin(timeValue) / /2.0f);

		// glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
		// glm::mat4 trans = glm::mat4(1.0f);

		// glUniform1f(glGetUniformLocation(default_shader, "time"), value);

		glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, texture);
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER EBO);
		// glBindVertexArray(VAO);

		glm::mat4 camera_pos = glm::mat4(1.0f);
		camera_pos = glm::translate(camera_pos, -glm::vec3(0.0f, 0.0f, 5.0f));

		glUniformMatrix4fv(glGetUniformLocation(default_shader, "camera_transform"), 1, GL_FALSE, glm::value_ptr(camera_pos));
		glUniformMatrix4fv(glGetUniformLocation(default_shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

		// for (unsigned int i = 0; i < 10; i++) {
		// 	glm::mat4 trans = glm::mat4(1.0f);
		// 	trans = glm::translate(trans, cubePositions[i]);
		// 	trans = glm::rotate(trans, glm::radians(timeValue * 90.0f * i), glm::vec3(0.5, 1, 0.2));
		// 	// trans = glm::translate(trans, glm::vec3(value, 0.0f, 0.0f));	
		// 	glUniformMatrix4fv(glGetUniformLocation(default_shader, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
		// 	glDrawArrays(GL_TRIANGLES, 0, 36);
		// }

		bsge_call_lua_render(L, delta_time);

		float calc_time = glfwGetTime() - current_frame;
		last_frame = current_frame;


		glfwSwapBuffers(window);
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			printf("[main.cpp] exit: OpenGL error: %i\n", err);
		}
	}
}
