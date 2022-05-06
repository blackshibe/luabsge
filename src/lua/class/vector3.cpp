#include "vector3.h"

int lua_bsge_new_vector3(lua_State* L) {
	lua_newtable(L);
	luaL_getmetatable(L, "Vector3");
	lua_setmetatable(L, -2);

	return 1;
}

const luaL_Reg bsge_lua_vector3_methods[] = {
	{"new", lua_bsge_new_vector3},
	{NULL, NULL},
};

void lua_bsge_init_vector3(lua_State* L) {
	// {__index = { ...bsge_lua_vector_methods }}
	luaL_newmetatable(L, "Vector3");
	luaL_setfuncs(L, bsge_lua_vector3_methods, 0);

	lua_newtable(L);
	lua_setfield(L, -2, "__index");

	lua_newtable(L);
	luaL_setfuncs(L, bsge_lua_vector3_methods, 0);
	lua_setglobal(L, "Vector3");
}
