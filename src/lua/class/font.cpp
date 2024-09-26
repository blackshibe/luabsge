#include "font.h"

int font_load(Font font, const char *path) {
	printf("Font is somehow loading\n");

	return 0;
}

int dbg_get_data(Font font) {
	printf("-- getting dbg data for font\n");
	printf("-- path: %s\n", font.path);

	return 0;
};

int get_info_for_char(lua_State *L) {
	const char *character = lua_tostring(L, 2);

	printf("[font.cpp] getting char info for %c\n", character[0]);

	int char_index = (int)character[0];
	lua_newtable(L);

	return 0;
}

void lua_bsge_init_font(sol::state &lua) {
	lua_State *L = lua.lua_state();
	auto load = [&L](Font *font, const char *path) {
		if (!freetype_load_font(*font, path)) {
			luax_push_error(L, "couldn't load font");
			return 1;
		}

		return 0;
	};

	lua.new_usertype<Font>("Font",
						   "load", load,
						   "get_info_for_char", get_info_for_char,
						   "dbg_get_data", dbg_get_data);
}
