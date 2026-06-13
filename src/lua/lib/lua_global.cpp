#include "lua_global.h"

#include "util/output.h"
#include "util/time.h"

#include <string>

static Output output;

namespace Lua {
	namespace global {
		std::string concat_arguments(lua_State *L) {
			int arguments = lua_gettop(L);
			std::string result;

			for (int i = 1; i <= arguments; i++) {
				size_t l;
				const char *s = luaL_tolstring(L, i, &l);
				if (i > 1)
					result += '\t';
				result.append(s, l);
				lua_pop(L, 1);
			}

			return result;
		}

		int warn(lua_State *L) {
			output.warn("%s", concat_arguments(L).c_str());
			return 0;
		}

		int print(lua_State *L) {
			output.info("%s", concat_arguments(L).c_str());
			return 0;
		}

		int now(lua_State *L) {
			lua_pushnumber(L, Lua::time::now());
			return 1;
		}
	}
}
