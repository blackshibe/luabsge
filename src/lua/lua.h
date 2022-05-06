#pragma once

#include <lua.hpp>

#include "../opengl/window.h"

int64_t now();

// adds a prefix to Lua print
// lbaselib.c
int lua_base_print(lua_State* L);
int lua_warn(lua_State* L);
int lua_print(lua_State* L);
int lua_now(lua_State* L);

extern BSGEWindow* context_window;
int bsge_lua_init_state(BSGEWindow* _window, lua_State* L);