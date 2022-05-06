#include <lua.hpp>

#include "../luax.h"
#include "../class/signal.h"
#include "../../opengl/freetype.h"
#include "../../include/colors.h"
#include "../class/mesh.h"
#include "../../opengl/window.h"
#include "../lua.h"

int lua_bsge_load_mesh(lua_State* L) {
	lua_bsge_new_mesh(L);
	return 1;
}

static const luaL_Reg bsge_meshloader_methods[] = {
	{"load_mesh", lua_bsge_load_mesh},
	{NULL, NULL},
};

void lua_bsge_connect_meshloader(BSGEWindow _window, lua_State* L) {

	luaL_newmetatable(L, "Meshloader");
	luaL_setfuncs(L, bsge_meshloader_methods, 0);

	// world.mesh.primitive = {}
	lua_newtable(L);
	lua_setfield(L, -2, "primitive");
	lua_setfield(L, -2, "meshloader");
}