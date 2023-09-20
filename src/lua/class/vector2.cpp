#include "vector2.h"

// TODO: REMOVE

int lua_bsge_new_vector2(lua_State* L) {
	lua_newtable(L);
	luaL_getmetatable(L, "Vector2");
	lua_setmetatable(L, -2);

	return 1;
}

const luaL_Reg bsge_lua_vector2_methods[] = {
	{"new", lua_bsge_new_vector2},
	{NULL, NULL},
};

void lua_bsge_init_vector2(lua_State* L) {
	// {__index = { ...bsge_lua_vector_methods }}
	luaL_newmetatable(L, "Vector2");
	luaL_setfuncs(L, bsge_lua_vector2_methods, 0);

	lua_newtable(L);
	lua_setfield(L, -2, "__index");

	lua_newtable(L);
	luaL_setfuncs(L, bsge_lua_vector2_methods, 0);
	lua_setglobal(L, "Vector2");
}
