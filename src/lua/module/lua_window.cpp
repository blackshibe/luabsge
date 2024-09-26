#include <lua.hpp>

#include "../lua.h"

BSGEWindow window;
int lua_bsge_window_quit(lua_State *L) {
	printf("The program appears to have been told to fuck off\n");
	glfwSetWindowShouldClose(window.window, true);

	return 0;
}

static const luaL_Reg bsge_window_methods[] = {
	{"quit", lua_bsge_window_quit},
	{NULL, NULL},
};

int lua_bsge_init_window(lua_State *L) {
	return 0;
}

int lua_bsge_connect_window(BSGEWindow _window, lua_State *L) {
	window = _window;

	luaL_newmetatable(L, "Window");
	luaL_setfuncs(L, bsge_window_methods, 0);

	// lua_pushstring(L, "i fuck men");
	// lua_setfield(L, -2, "ass");
	lua_setfield(L, -2, "window");

	return 0;
}