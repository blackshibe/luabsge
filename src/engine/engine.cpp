#include "engine/engine.h"
#include "rendering/window.h"

static Output output;

// config.lua and main.lua are split for cleanliness and because of legacy code
// perhaps there should be a manifest of what to run when the project starts

EngineInstance::EngineInstance() {
	Lua::object::window::init(lua);

	// get Lua ready before executing it
	lua.open_libraries();
	lua.set_function("now", Lua::global::now);
	lua.set_function("print", Lua::global::print);
	lua.set_function("warn", Lua::global::warn);

	// TODO shouldn't be globals at all
	lua["BSGE_PLATFORM"] = "NATIVE";
	lua["BSGE_RENDERER"] = "Vulkan"; // todo assign autonomously
	lua["BSGE_VERSION"] = Engine::VERSION;

	Lua::util::run_script(lua, "config.lua");
}

EngineInstance::~EngineInstance() = default;

void EngineInstance::preflight() {
	output.info("preflight");

	// get the window ready
	window = std::make_unique<VulkanWindowInstance>(*this);

	// first user code run
	if (Lua::util::run_script(lua, "entry.lua")) {
		// glfwSetFramebufferSizeCallback(window->glfw_window, window_resize);

		// if (window->status == -1) {
		// 	return EXIT_FAILURE;
		// }

		// window_resize(window->window, window->width, window->height);
		// window->render_loop_init();
	} else {
		throw std::runtime_error("Lua error when starting game");
	}
}

void EngineInstance::start() {
	window->render_loop_init();
}