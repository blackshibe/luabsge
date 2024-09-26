#include <lua.hpp>

#include "../../include/colors.h"
#include "../../opengl/freetype.h"
#include "../class/signal.h"
#include "../luax.h"

#include "../../opengl/window.h"
#include "../lua.h"

int fuck_function;
int fuck(lua_State *L) {
	printf("%s[rendering.cpp] error in luarender:\n%s\n", ANSI_RED, lua_tostring(L, -1));

	luaL_traceback(L, L, NULL, 2);
	printf("%s%s\n", lua_tostring(L, -1), ANSI_NC);
	return 0;
}

int bsge_call_lua_render(sol::state *lua, float delta_time) {
	lua_State *L = lua->lua_state();

	luaL_getmetatable(L, "Rendering");
	lua_getfield(L, -1, "step");
	lua_getfield(L, -1, "fire");
	lua_remove(L, -2);
	lua_insert(L, -2);
	lua_getfield(L, -1, "step");
	lua_remove(L, -2);

	// Rendering.step:fire(delta_time: number)
	lua_pushnumber(L, delta_time);
	lua_pcall(L, 2, 0, fuck_function);

	return 0;
}

int lua_bsge_init_rendering(sol::state &lua) {
	lua_State *L = lua.lua_state();

	lua_pushcfunction(L, fuck);
	fuck_function = lua_gettop(L);
	return 0;
}

int lua_bsge_connect_rendering(BSGEWindow _window, sol::state &lua) {
	lua_State *L = lua.lua_state();

	luaL_newmetatable(L, "Rendering");

	lua_bsge_new_signal(L);
	lua_setfield(L, -2, "step");
	lua_setfield(L, -2, "rendering");
	return 0;
}