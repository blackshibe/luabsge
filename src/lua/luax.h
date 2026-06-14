#pragma once

#include <lua.hpp>
#include <sol/forward.hpp>

#include <sol/sol.hpp>

namespace Lua::util {
	bool run_script(sol::state &lua, const char *filename);
}
