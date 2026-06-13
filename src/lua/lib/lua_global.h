#pragma once

#include <lua.hpp>
#include <string>
#include "util/output.h"

namespace Lua {
	namespace global {
		std::string concat_arguments(lua_State *L);

		int warn(lua_State *L);
		int print(lua_State *L);
		int now(lua_State *L);
	}
}
