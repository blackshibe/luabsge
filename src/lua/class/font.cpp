#include "font.h"

// Fonts are a loaded asset. They have no Lua-facing constructor.

int lua_bsge_index(lua_State* L) {
	// this, index
	printf("[font.cpp] newindex\n");
	printf(lua_tostring(L, 2));
	printf("\n");

	return 0;
}

int lua_bsge_newindex(lua_State* L) {
	// this, index, value
	printf("[font.cpp] newindex\n");
	printf(lua_tostring(L, 2));
	printf("\n");

	return 0;
}

const luaL_Reg bsge_lua_font_metatable_access[] = {
	{"__index", lua_bsge_index},
	{"__newindex", lua_bsge_newindex},

	{NULL, NULL},
};

void lua_bsge_init_font(lua_State* L) {
	luaL_newmetatable(L, "Font");
	luaL_setfuncs(L, bsge_lua_font_metatable_access, 0);
}
