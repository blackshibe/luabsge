#include "textlabel.h"

int textlabel_render(lua_State* L) {
	struct Textlabel* label = (Textlabel*)lua_touserdata(L, 1);

	freetype_render(*label, freetype_text_shader);

	return 0;
}

// this, index
int textlabel_index(lua_State* L) {
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "render") == 0) {
		lua_pushcclosure(L, textlabel_render, 0);
		return 1;
	} else if (strcmp(index, "text") == 0) {
		struct Textlabel* label = (Textlabel*)lua_touserdata(L, 1);
		lua_pushstring(L, label->text);
		return 1;
	}

	return 0;
}

// this, index, value
int textlabel_newindex(lua_State* L) {
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "font") == 0) {
		struct Textlabel* label = (Textlabel*)lua_touserdata(L, 1);
		label->font = *(Font*)lua_touserdata(L, 3);
	} else if (strcmp(index, "text") == 0) {
		struct Textlabel* label = (Textlabel*)lua_touserdata(L, 1);
		const char* text = lua_tostring(L, 3);
		label->text = strdup(text);
	}

	return 0;
}

const luaL_Reg bsge_lua_textlabel_metatable_access[] = {
	{"__index", textlabel_index},
	{"__newindex", textlabel_newindex},

	{NULL, NULL},
};

int lua_bsge_instance_textlabel(lua_State* L) {
	struct Textlabel* font = (Textlabel*)lua_newuserdata(L, sizeof(Textlabel));
	font->position = glm::vec2(10.0f, 10.0f);
	font->color = glm::vec3(1.0f, 1.0f, 1.0f);
	font->scale = 0.2f;
	font->text = "Hello world";

	luaL_getmetatable(L, "Textlabel");
	lua_setmetatable(L, -2);

	return 1;
}

void lua_bsge_init_textlabel(lua_State* L) {
	luaL_newmetatable(L, "Textlabel");
	luaL_setfuncs(L, bsge_lua_textlabel_metatable_access, 0);

	// todo: replace with a universal constructor
	lua_register(L, "_temp_TextLabel", lua_bsge_instance_textlabel);
}
