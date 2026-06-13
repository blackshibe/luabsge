#include "font.h"

void lua_bsge_init_font(sol::state &lua) {
	lua_State *L = lua.lua_state();
	auto load = [&L](Font *font, const char *path) {
		if (!freetype_load_font(font, path)) {
			luax_push_error(L, "couldn't load font");
			return 1;
		}

		return 0;
	};

	lua.new_usertype<Font>("Font",
						   "load", load);
}
