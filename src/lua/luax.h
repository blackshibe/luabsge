#pragma once

#include <lua.hpp>


void luax_print_error(lua_State* L);
void luax_push_error(lua_State* L, const char* error);
void luax_call_global_if_exists(lua_State* L, const char* global, int args);
bool luax_run_script(lua_State* L, const char* filename);