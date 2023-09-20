#include "color.h"

int lua_bsge_new_color(lua_State *L) {
    lua_newtable(L);
    luaL_getmetatable(L, "Color");
    lua_setmetatable(L, -2);

    return 1;
}

const luaL_Reg bsge_lua_color_methods[] = {
    {"new", lua_bsge_new_color},
	{NULL, NULL},
};

void lua_bsge_init_color(lua_State *L) {
    // {__index = { ...bsge_lua_vector_methods }}
    luaL_newmetatable(L, "Color");
    luaL_setfuncs(L, bsge_lua_color_methods, 0);

    lua_newtable(L);
    lua_setfield(L, -2, "__index");

    lua_newtable(L);
    luaL_setfuncs(L, bsge_lua_color_methods, 0);
    lua_setglobal(L, "Color");
}
