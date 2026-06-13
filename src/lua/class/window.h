#pragma once

#include <sol/sol.hpp>
#include "rendering/window.h"

namespace Lua {
	namespace object {
		namespace window {
			void init(sol::state &lua);
		}
	}
}
