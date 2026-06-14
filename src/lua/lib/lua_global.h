#pragma once

#include "util/output.h"
#include <lua.hpp>
#include <sol/sol.hpp>
#include <string>

namespace Lua {
	void init(sol::state &lua);

	std::string concat_arguments(lua_State *L);

	int warn(lua_State *L);
	int print(lua_State *L);
	int now(lua_State *L);
}
