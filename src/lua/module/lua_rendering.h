#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>

#include "../../opengl/window.h"

int lua_bsge_init_rendering(sol::state &lua);
int bsge_call_lua_render(sol::state *L, float delta_time);
int lua_bsge_connect_rendering(BSGEWindow _window, sol::state &lua);