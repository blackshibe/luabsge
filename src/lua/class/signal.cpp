#include "signal.h"

int lua_bsge_new_signal(lua_State* L) {
	lua_newtable(L);
	lua_newtable(L);
	lua_setfield(L, -2, "connections");

	luaL_setmetatable(L, "Signal");

	return 1;
}

int lua_bsge_signal_connect(lua_State* L) {
	// i don't know how to move stack elements
	int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// table.insert(self.connections, ...)
	lua_getglobal(L, "table");
	lua_getfield(L, -1, "insert");
	lua_getfield(L, -3, "connections");
	lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
	lua_call(L, 2, 0);

	return 0;
};

int lua_bsge_signal_fire(lua_State* L) {
	// for i = 1, #self.methods do
	//    self.methods[i](...)
	// end

	int args = lua_gettop(L);

	lua_getfield(L, -args, "connections");
	lua_len(L, -1);
	int len = lua_tointeger(L, -1);
	lua_pop(L, 1);

	for (int i = 1; i <= len; i++) {
		lua_rawgeti(L, -1, i);
		for (int i = 0; i < args; i++) lua_pushvalue(L, 2 + i);
		lua_call(L, args, 0);
	}

	return 0;
};

const luaL_Reg bsge_lua_signal_methods[] = {
	{"connect", lua_bsge_signal_connect},
	{"fire", lua_bsge_signal_fire},
	{NULL, NULL},
};

void lua_bsge_init_signal(lua_State* L) {
	// {__index = { ...bsge_lua_signal_methods }}
	luaL_newmetatable(L, "Signal");
	lua_newtable(L);
	luaL_setfuncs(L, bsge_lua_signal_methods, 0);
	lua_setfield(L, -2, "__index");
}
