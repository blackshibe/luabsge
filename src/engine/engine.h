#pragma once

#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <memory>

#include "util/output.h"
#include "lua/lib/lua_global.h"
#include "lua/luax.h"
#include "lua/class/window.h"

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
