#pragma once

#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <memory>

#include "util/output.h"
#include "lua/lib/lua_global.h"
#include "lua/luax.h"
#include "lua/class/window.h"

#define BSGE_VERSION_MAJOR "0"
#define BSGE_VERSION_MINOR "0"
#define BSGE_VERSION_PATCH "vulkan"

namespace Engine {
    static constexpr const char *VERSION = BSGE_VERSION_MAJOR "." BSGE_VERSION_MINOR "-" BSGE_VERSION_PATCH;

}

// avoids circular dependency
class WindowInstance;

class EngineInstance {
public:

    entt::registry registry;
	sol::state lua;
    std::unique_ptr<WindowInstance> window;

    EngineInstance();
    ~EngineInstance();

    void preflight();
    void start();
};
