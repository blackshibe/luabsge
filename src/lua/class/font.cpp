#include "font.h"

int font_load(lua_State* L) {
	struct Font* font = (Font*)lua_touserdata(L, 1);

	if (!freetype_load_font(font, lua_tostring(L, 2))) {
		luax_push_error(L, "couldn't load font");
		return 1;
	}

	return 0;
}

int get_info_for_char(lua_State* L) {
	struct Font* font = (Font*)lua_touserdata(L, 1);
	const char* character = lua_tostring(L, 2);

	printf("[font.cpp] getting char info for %c\n", character[0]);

	int char_index = (int)character[0];
	lua_newtable(L);

	// font->data[(int)character[0]].advance;

	// if (!freetype_load_font(font, lua_tostring(L, 2))) {
	// 	luax_push_error(L, "couldn't load font");
	// 	return 1;
	// }

	return 0;
}

int font_index(lua_State* L) {
	// this, index
	printf("[font.cpp] index\n");
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "load") == 0) {
		lua_pushcclosure(L, font_load, 0);
		return 1;
	} else if (strcmp(index, "get_info_for_char") == 0) {
		lua_pushcclosure(L, get_info_for_char, 0);
		return 1;
	}

	return 0;
}

int font_newindex(lua_State* L) {
	printf("[font.cpp] newindex\n");

	return 0;
}

const luaL_Reg bsge_lua_font_metatable_access[] = {
	{"__index", font_index},
	{"__newindex", font_newindex},

	{NULL, NULL},
};

int lua_bsge_instance_font(lua_State* L) {
	struct Font* font = (Font*)lua_newuserdata(L, sizeof(Font));
	luaL_getmetatable(L, "Font");
	lua_setmetatable(L, -2);

	return 1;
}

void lua_bsge_init_font(lua_State* L) {
	luaL_newmetatable(L, "Font");
	luaL_setfuncs(L, bsge_lua_font_metatable_access, 0);

	// todo: replace with a universal constructor
	lua_register(L, "_temp_Font", lua_bsge_instance_font);
}
