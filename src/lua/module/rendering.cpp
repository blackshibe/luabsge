#include <lua.hpp>


#include "../luax.h"
#include "../class/signal.h"
#include "../../opengl/freetype.h"
#include "../../include/colors.h"

#include "../../opengl/window.h"
#include "../lua.h"

int fuck_function;
int fuck(lua_State* L) {
	printf("%s[rendering.cpp] error in luarender:\n%s\n", ANSI_RED, lua_tostring(L, -1));

	luaL_traceback(L, L, NULL, 2);
	printf("%s%s\n", lua_tostring(L, -1), ANSI_NC);
	return 0;
}

int bsge_call_lua_render(lua_State* L, float delta_time) {

	luaL_getmetatable(L, "Rendering");
	lua_getfield(L, -1, "step");
	lua_getfield(L, -1, "fire");
	lua_remove(L, -2);
	lua_insert(L, -2);
	lua_getfield(L, -1, "step");
	lua_remove(L, -2);
	lua_pushnumber(L, delta_time);
	lua_pcall(L, 2, 0, fuck_function);

	return 0;
}

int lua_bsge_draw_font(lua_State* L) {

	const char* text = lua_tostring(L, 1);
	struct Font* font = (Font*)lua_touserdata(L, 2);

	freetype_render(
		text,
		font,
		freetype_text_shader,
		glm::vec3(1.0f, 1.0f, 1.0f)
	);

	return 0;
}

static const luaL_Reg bsge_rendering_methods[] = {
	{"draw_font", lua_bsge_draw_font},
	{NULL, NULL},
};

int lua_bsge_init_rendering(lua_State* L) {
	lua_pushcfunction(L, fuck);
	fuck_function = lua_gettop(L);
}

int lua_bsge_connect_rendering(BSGEWindow _window, lua_State* L) {
	luaL_newmetatable(L, "Rendering");
	luaL_setfuncs(L, bsge_rendering_methods, 0);

	lua_bsge_new_signal(L);
	lua_setfield(L, -2, "step");
	lua_setfield(L, -2, "rendering");
	return 0;
}