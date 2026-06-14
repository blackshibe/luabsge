#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>

namespace Lua::global::imgui {
	void init(sol::state &lua);
}
