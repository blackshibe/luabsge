#pragma once

#include <lua.hpp>
#include <sol/sol.hpp>

namespace Lua {
namespace global {
namespace imgui {
    void init(sol::state &lua);
}
}
}