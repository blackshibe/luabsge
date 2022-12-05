#pragma once

#include <lua.hpp>

int lua_bsge_init_window(lua_State* L);
int bsge_call_lua_window(lua_State *L, float delta_time);
int lua_bsge_connect_window(BSGEWindow _window, lua_State* L);