#pragma once

#include "../lua_state.h"
#include <lua.hpp>

#include "../../include/imgui/imgui.h"

void lua_bsge_init_imgui_bindings(sol::state &lua);