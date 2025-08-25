#include <lua.hpp>

#include "../include/colors.h"
#include <cstdio>
#include <lauxlib.h>

// The error message is on top of the stack.
// Fetch it, print it and then pop it off the stack.
void luax_print_error(lua_State* L) {
	printf(lua_tostring(L, -1));
	lua_pop(L, 1);
}

void luax_push_error(lua_State* L, const char* error) {
	lua_pushstring(L, error);
	lua_error(L);
}

bool luax_run_script(lua_State* L, const char* filename) {
	printf("[luax.cpp] compiling %s\n", filename);

	int result = luaL_loadfile(L, filename);

	if (result == LUA_OK) {
		result = lua_pcall(L, 0, 0, 0);
		if (result != LUA_OK) {
			printf("%s[main.cpp] ", ANSI_RED);
			luax_print_error(L);
			printf("%s\n", ANSI_NC);
			
			return false;
		}
	} else {
		printf("%s[main.cpp] ", ANSI_RED);
		luax_print_error(L);
		printf("%s\n", ANSI_NC);

		return false;
	}

	return true;
}