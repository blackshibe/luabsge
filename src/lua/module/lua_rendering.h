#pragma once

#include <lua.hpp>

#include "../../opengl/window.h"

int lua_bsge_init_rendering(lua_State* L);
int bsge_call_lua_render(lua_State *L, float delta_time);
int lua_bsge_draw_font(lua_State* L);
int lua_bsge_connect_rendering(BSGEWindow _window, lua_State* L);