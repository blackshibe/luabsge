#pragma once

#include "entt/entity/fwd.hpp"
#include "resource/resource.h"
#include <entt/entt.hpp>
#include <memory>
#include <sol/sol.hpp>

#define BSGE_VERSION_MAJOR "0"
#define BSGE_VERSION_MINOR "0"
#define BSGE_VERSION_PATCH "0"
#define BSGE_VERSION_BRANCH "vulkan"

namespace Engine {
	static constexpr const char *VERSION = BSGE_VERSION_MAJOR "." BSGE_VERSION_MINOR "." BSGE_VERSION_PATCH "-" BSGE_VERSION_BRANCH;
}

// avoids circular dependency
class WindowInstance;

namespace Lua::instance {
	class Instance;
}

class EngineInstance {
  public:
	// scene
	std::unique_ptr<Lua::instance::Instance> scene_root;
	entt::registry registry;

	// data
	Resource::Bank resources;

	// lua
	sol::state lua;

	// rendering
	std::unique_ptr<WindowInstance> window;

	EngineInstance();
	~EngineInstance();

	void preflight();
	void start();
};
