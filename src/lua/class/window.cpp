#include "lua/class/window.h"
#include "rendering/window/window.h"

namespace Lua::object::window {
	void init(sol::state &lua) {
		lua.new_usertype<WindowConfiguration>(
		    "WindowConfiguration",
		    sol::constructors<WindowConfiguration(int, int, const char *)>(),
		    "width", &WindowConfiguration::width,
		    "height", &WindowConfiguration::height,
		    "name", &WindowConfiguration::name);

		lua.new_usertype<WindowInstance>(
		    "WindowInstance",
		    sol::no_constructor);
	}
}
