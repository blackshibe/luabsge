#include "window.h"
#include "engine/engine.h" // full EngineInstance definition (forward-declared in window.h)
#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_vulkan.h"

// #if USE_EMSCRIPTEN
// #include <emscripten.h>
// #include <emscripten/html5.h>
// #include <GLFW/emscripten_glfw3.h>

// // Static pointer for emscripten main loop callback
// static BSGEWindow* g_window_instance = nullptr;

// // Static callback function for emscripten
// static void emscripten_render_loop_callback() {
// 	if (g_window_instance) {
// 		g_window_instance->render_loop();
// 	}
// }
// #endif

void resize_callback(GLFWwindow *window, int width, int height) {
	((WindowInstance *)glfwGetWindowUserPointer(window))->callback_resized(width, height);
}

void error_callback(int error_code, const char *description) {
	printf("[main.cpp] glfw error callback called because '%s'\n", description);
}

WindowInstance::WindowInstance(EngineInstance &engine) : engine(engine) {}

bool WindowInstance::render_loop() {
	return false;
}

void WindowInstance::render_pass() {}

void WindowInstance::render_loop_init() {}

void WindowInstance::callback_resized(int width, int height) {}

void WindowInstance::callback_focused(int focused) {}

VulkanWindowInstance::VulkanWindowInstance(EngineInstance &engine) : WindowInstance(engine) {
	glfwInit();
	glfwSetErrorCallback(error_callback);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	WindowConfiguration config(900, 400, "LuaBSGE");
	sol::optional<sol::table> bsge = engine.lua["BSGE"];
	if (bsge) {
		sol::optional<WindowConfiguration> window_config = (*bsge)["window_configuration"];
		if (window_config) config = *window_config;
	}

	glfw_window = glfwCreateWindow(config.width, config.height, config.name, NULL, NULL);
	glfwSetWindowUserPointer(glfw_window, this);
	vulkan_renderer = std::make_unique<Vulkan::Renderer>(engine, glfw_window);
}

// pumps events and draws one frame; returns false once the window wants to close
bool VulkanWindowInstance::render_loop() {
	glfwPollEvents();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	// let Lua run its per-frame render logic, if the game defined one
	// TODO doesn't belong in window
	sol::optional<sol::protected_function> render = engine.lua["TODO_RENDER"];
	if (render) {
		sol::protected_function_result result = (*render)();
		if (!result.valid()) {
			sol::error err = result;
			printf("[window.cpp] TODO_RENDER error: %s\n", err.what());
		}
	}

	// finalize the imgui frame (builds the draw data) before the renderer records it
	ImGui::Render();

	vulkan_renderer->draw();

	return !glfwWindowShouldClose(glfw_window);
}

// blocking main loop: keep drawing until the window closes
void VulkanWindowInstance::render_loop_init() {
	while (render_loop()) {
	}
}

// void BSGEWindow::init() {
// 	if (this->window == NULL) {
// 		printf("[window.cpp] exit: failed to create GLFW window\n");
// 		glfwTerminate();
// 		status = -1;
// 		return;
// 	}

// 	glfwMakeContextCurrent(this->window);
// 	glfwSetWindowUserPointer(this->window, this);

// 	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
// 		printf("[window.cpp] exit: failed to initialize GLAD\n");
// 		glfwTerminate();
// 		status = -1;
// 		return;
// 	}

// 	// Enable docking
// 	ImGuiIO& io = ImGui::GetIO();
// 	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

// 	// Explose Lua render_pass
// 	(*lua)["World"]["rendering"]["render_pass"] = [this]() {
// 		this->render_pass();
// 	};

// 	#if USE_EMSCRIPTEN

// 	// Set up static instance for emscripten callback
// 	g_window_instance = this;
// 	emscripten_glfw_make_canvas_resizable(window, "#bsge-canvas-container", nullptr);
// 	emscripten_set_main_loop(emscripten_render_loop_callback, 0, 1);

// 	#else

// 	while (!should_break && !glfwWindowShouldClose(this->window)) {
// 		this->render_loop();
// 	}

// 	#endif
// }

// bool BSGEWindow::render_loop() {
// 	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
// 		glfwSetWindowShouldClose(window, true);
// 	}

// 	// frame start
// 	float current_frame = glfwGetTime();
// 	float delta_time = current_frame - last_frame;

// 	// update input system
// 	update_mouse_input();

// 	// get camera projection and coordinates from the Lua context using sol2
// 	sol::table world = (*lua)["World"];
// 	sol::table rendering = world["rendering"];
// 	sol::optional<BSGECameraMetadata*> camera_opt = rendering["camera"];

// 	if (!camera_opt) {
// 		printf("%s", ANSI_RED);
// 		printf("[window.cpp] no camera defined!\n");
// 		printf("[window.cpp] as there is currently no fallback, the window will shut down.\n");
// 		printf("%s", ANSI_NC);

// 		glfwSetWindowShouldClose(window, true);
// 		#if USE_EMSCRIPTEN
// 		return true;
// 		#else
// 		return false;
// 		#endif
// 	}

// 	set_current_buffer_dimensions(get_window_dimensions());

// 	bsge_call_lua_render(lua, delta_time);

// 	// physics system update
// 	BSGE::Physics::update(delta_time);

// 	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

// #ifndef USE_EMSCRIPTEN // web doesn't implement this
// 	glfwSwapBuffers(window);
// #endif
// 	glfwPollEvents();

// 	const GLenum err = glGetError();
// 	if (err != GL_NO_ERROR) {
// 		printf("[main.cpp] exit: OpenGL error: %i\n", err);
// 	}

// 	#if USE_EMSCRIPTEN
// 	if (glfwWindowShouldClose(window) || should_break) {
// 		emscripten_cancel_main_loop();
// 	}
// 	return true;
// 	#else
// 	return !glfwWindowShouldClose(window) && !should_break;
// 	#endif
// }

// void BSGEWindow::render_pass() {
// 	// Get camera position for transparency sorting
// 	sol::table world = (*lua)["World"];
// 	sol::table rendering = world["rendering"];
// 	sol::optional<BSGECameraMetadata*> camera_opt = rendering["camera"];
// 	glm::vec3 camera_position = glm::vec3(0.0f);

// 	if (camera_opt) {
// 		BSGECameraMetadata* camera = camera_opt.value();
// 		camera_position = glm::vec3(camera->transform[3]);
// 	}

// 	// https://skypjack.github.io/2019-08-20-ecs-baf-part-4-insights
// 	// TODO: only sort objects updated this frame
// 	std::unordered_map<entt::entity, int> depths;
// 	std::unordered_map<entt::entity, glm::mat4> transforms;
// 	std::unordered_map<entt::entity, float> distances;

// 	auto calculate_depth = [this, &depths](const entt::entity entity, auto&& self) -> int {
// 		if (depths.find(entity) != depths.end()) {
// 			return depths[entity];
// 		}

// 		EcsObjectComponent* object = registry.try_get<EcsObjectComponent>(entity);
// 		if (!object || object->parent == entt::null) {
// 			depths[entity] = 0;
// 			return 0;
// 		}

// 		int parent_depth = self(object->parent, self);
// 		depths[entity] = parent_depth + 1;

// 		return parent_depth + 1;
// 	};

// 	registry.view<EcsObjectComponent>().each([&calculate_depth](const entt::entity entity, const EcsObjectComponent &object_component) {
// 		calculate_depth(entity, calculate_depth);
// 	});

// 	registry.sort<EcsObjectComponent>([&depths](const entt::entity lhs, const entt::entity rhs) {
// 		return depths[lhs] < depths[rhs];
// 	});

// 	// Calculate transforms and distances for all entities first
//     auto view = registry.view<EcsObjectComponent>();
// 	std::vector<entt::entity> opaque_entities;
// 	std::vector<entt::entity> transparent_entities;

// 	view.each([this, &transforms, &depths, &distances, &camera_position, &opaque_entities, &transparent_entities](entt::entity entity, EcsObjectComponent &object_component) {
// 		// handle hierarchy
// 		glm::mat4 final_transform;

// 		if (object_component.parent != entt::null) {
// 			EcsObjectComponent* parent_object = registry.try_get<EcsObjectComponent>(object_component.parent);
// 			final_transform = transforms[object_component.parent] * object_component.transform;
// 		} else {
// 			final_transform = object_component.transform;
// 		}

// 		EcsPhysicsComponent* physics_component = registry.try_get<EcsPhysicsComponent>(entity);
// 		if (physics_component) {
// 			glm::mat4 physics_transform = BSGE::Physics::get_body_transform(physics_component->body_id);
// 			glm::vec3 scale = glm::vec3(
// 				glm::length(glm::vec3(object_component.transform[0])),
// 				glm::length(glm::vec3(object_component.transform[1])),
// 				glm::length(glm::vec3(object_component.transform[2]))
// 			);
// 			final_transform = physics_transform * glm::scale(glm::mat4(1.0f), scale);
// 		}

// 		transforms[entity] = final_transform;

// 		EcsMeshComponent* mesh_component = registry.try_get<EcsMeshComponent>(entity);
// 		if (!mesh_component) {
// 			return; // some things aren't renderable
// 		}

// 		// Calculate distance from camera for transparency sorting
// 		glm::vec3 object_position = glm::vec3(final_transform[3]);
// 		distances[entity] = glm::distance(camera_position, object_position);

// 		// Separate transparent and opaque objects
// 		bool is_transparent = mesh_component->mesh.color.w < 1.0f;
// 		if (is_transparent) {
// 			transparent_entities.push_back(entity);
// 		} else {
// 			opaque_entities.push_back(entity);
// 		}
// 	});

// 	// Sort opaque objects by hierarchy depth (front to back for depth testing efficiency)
// 	std::sort(opaque_entities.begin(), opaque_entities.end(), [&depths](const entt::entity lhs, const entt::entity rhs) {
// 		return depths[lhs] < depths[rhs];
// 	});

// 	// Sort transparent objects by distance from camera (back to front for proper alpha blending)
// 	std::sort(transparent_entities.begin(), transparent_entities.end(), [&distances](const entt::entity lhs, const entt::entity rhs) {
// 		return distances[lhs] > distances[rhs];
// 	});

// 	glDisable(GL_BLEND);
// 	glDepthMask(GL_TRUE);

// 	// Render opaque objects first
// 	for (entt::entity entity : opaque_entities) {
// 		EcsMeshComponent mesh_component = registry.get<EcsMeshComponent>(entity);
// 		glm::mat4 final_transform = transforms[entity];

// 		glBindVertexArray(0);
// 		glBindBuffer(GL_ARRAY_BUFFER, 0);
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

// 		// Get optional texture component
// 		EcsMeshTextureComponent* texture_component = registry.try_get<EcsMeshTextureComponent>(entity);

// 		if (texture_component) {
// 			mesh_component.mesh.texture = texture_component->texture.id;
// 			mesh_component.color = mesh_component.mesh.color;
// 		} else {
// 			mesh_component.mesh.texture = 0;
// 		}

// 		mesh_render(*lua, final_transform, mesh_component.mesh);
// 	}

// 	glEnable(GL_BLEND);
// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// 	glDepthMask(GL_FALSE);

// 	// Render transparent objects last
// 	for (entt::entity entity : transparent_entities) {
// 		EcsMeshComponent mesh_component = registry.get<EcsMeshComponent>(entity);
// 		glm::mat4 final_transform = transforms[entity];

// 		glBindVertexArray(0);
// 		glBindBuffer(GL_ARRAY_BUFFER, 0);
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

// 		EcsMeshTextureComponent* texture_component = registry.try_get<EcsMeshTextureComponent>(entity);

// 		if (texture_component) {
// 			mesh_component.mesh.texture = texture_component->texture.id;
// 			mesh_component.color = mesh_component.mesh.color;
// 		} else {
// 			mesh_component.mesh.texture = 0;
// 		}

// 		mesh_render(*lua, final_transform, mesh_component.mesh);
// 	}
