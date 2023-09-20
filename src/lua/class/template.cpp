#include "template.h"

int template_index(lua_State* L) {
	// this, index
	printf("[template.cpp] newindex\n");

	return 0;
}

int template_newindex(lua_State* L) {
	printf("[template.cpp] newindex\n");

	return 0;
}

const luaL_Reg bsge_lua_template_metatable_access[] = {
	{"__index", template_index},
	{"__newindex", template_newindex},

	{NULL, NULL},
};

int lua_bsge_instance_template(lua_State* L) {
	lua_newuserdata(L, 1);
	luaL_getmetatable(L, "Template");
	lua_setmetatable(L, -2);

	return 1;
}

void lua_bsge_init_template(lua_State* L) {
	luaL_newmetatable(L, "Template");
	luaL_setfuncs(L, bsge_lua_template_metatable_access, 0);

	// todo: replace with a universal constructor
	lua_register(L, "_temp_Template", lua_bsge_instance_template);
}
