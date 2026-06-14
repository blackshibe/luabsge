#include "engine/engine.h"
#include "rendering/window/window.h"

#include "include/imgui/imgui.h"
#include "include/implot/implot.h"
#include "lua/class/window.h"
#include "lua/lib/lua_global.h"
#include "lua/lib/lua_gltf.h"
#include "lua/lib/lua_imgui.h"
#include "lua/lib/lua_instance.h"
#include "lua/luax.h"
#include "util/output.h"
#include <memory>

static Output output;

// config.lua and main.lua are split for cleanliness and because of legacy code
// perhaps there should be a manifest of what to run when the project starts

EngineInstance::EngineInstance() {
	output.mark();

	Lua::init(lua);
	Lua::object::window::init(lua);
	Lua::imgui::init(lua);
	Lua::instance::init(this, lua);
	Lua::gltf::init(this, lua);

	// TODO shouldn't be globals at all
	lua["BSGE_PLATFORM"] = "NATIVE";
	lua["BSGE_RENDERER"] = "Vulkan"; // todo assign autonomously
	lua["BSGE_VERSION"] = Engine::VERSION;

	scene_root = std::make_unique<Lua::instance::Instance>("Scene");
	lua["Scene"] = scene_root.get();

	Lua::util::run_script(lua, "config.lua");
}

EngineInstance::~EngineInstance() = default;

void EngineInstance::preflight() {
	output.mark();

	// get the window ready; the renderer's init_imgui creates the ImGui context
	// and binds the GLFW/Vulkan backends to it
	window = std::make_unique<VulkanWindowInstance>(*this);

	// configure that single context (a second CreateContext here would shadow it
	// and the docking flag would land on the wrong context)
	ImPlot::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
	output.mark();
	window->render_loop_init();
}