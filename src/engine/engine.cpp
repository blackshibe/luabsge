#include "engine/engine.h"
#include "rendering/window.h"

static Output output;

EngineInstance::EngineInstance() {
	// needed for config.lua to register WindowConfiguration
	Lua::object::window::init(lua);

	Lua::util::run_script(lua, "config.lua");
}

EngineInstance::~EngineInstance() = default;

void EngineInstance::preflight() {
	output.info("preflight");

	// get Lua ready before creating the window
	lua_State *L = lua.lua_state();

	lua.open_libraries();
	lua.set_function("now", Lua::global::now);
	lua.set_function("print", Lua::global::print);
	lua.set_function("warn", Lua::global::warn);

	// TODO shouldn't be globals at all
	lua["BSGE_PLATFORM"] = "NATIVE";
	lua["BSGE_RENDERER"] = "Vulkan"; // todo assign autonomously
	lua["BSGE_VERSION"] = "VERSION"; // todo global constructor for this

	// object init
	// ...

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