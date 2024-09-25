#include "font.h"

struct LuaFont : Font {
};

int font_load(lua_State *L) {
	printf("Font is somehow loading\n");

	struct LuaFont *font = (LuaFont *)lua_touserdata(L, 1);

	if (!freetype_load_font(font, lua_tostring(L, 2))) {
		luax_push_error(L, "couldn't load font");
		return 1;
	}

	return 0;
}

int get_info_for_char(lua_State *L) {
	const char *character = lua_tostring(L, 2);

	printf("[font.cpp] getting char info for %c\n", character[0]);

	int char_index = (int)character[0];
	lua_newtable(L);

	return 0;
}

void lua_bsge_init_font(sol::state &lua) {
	lua.new_usertype<LuaFont>("Font",
							  "load", font_load,
							  "get_info_for_char", get_info_for_char);
}
