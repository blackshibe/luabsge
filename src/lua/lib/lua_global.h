#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>
#include <string>
#include "util/output.h"

namespace Lua {
	namespace global {
		void init(sol::state &lua);

		std::string concat_arguments(lua_State *L);

		int warn(lua_State *L);
		int print(lua_State *L);
		int now(lua_State *L);
	}
}
