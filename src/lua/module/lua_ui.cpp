#include "lua_ui.h"

int lua_bsge_load_font(lua_State* L) {

	// struct Textlabel* font = (Textlabel*)lua_newuserdata(L, sizeof(Textlabel));
	// font->position = glm::vec2(0.0f, 0.0f);
	// font->scale = 1.0f;

	// if (!freetype_load_font(font, lua_tostring(L, 1))) {
	// 	luax_push_error(L, "couldn't load font");
	// 	return 1;
	// }

	// luaL_getmetatable(L, "Font");
	// lua_setmetatable(L, -2);

	return 1;
}

int lua_bsge_load_image(lua_State* L) {
	return 0;
	// bsgeImage* texture = (bsgeImage*)lua_newuserdata(L, sizeof(bsgeImage));

	// texture->width = 0;
	// texture->height = 0;
	// texture->num_channels = 0;

	// stbi_set_flip_vertically_on_load(true);

	// unsigned char* data = stbi_load(
	// 	lua_tostring(L, 1),
	// 	&texture->width,
	// 	&texture->height,
	// 	&texture->num_channels,
	// 	STBI_rgb_alpha
	// );

	// if (data) {
	// 	printf("[main.cpp] image loaded\n");
	// 	printf("id: %i\n", texture->id);
	// 	printf("width: %i\n", texture->width);
	// 	printf("height: %i\n", texture->height);

	// 	glGenTextures(1, &texture->id);
	// 	glBindTexture(GL_TEXTURE_2D, texture->id);

	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	// 	glGenerateMipmap(GL_TEXTURE_2D);

	// 	stbi_image_free(data);
	// }
	// else {
	// 	luax_push_error(L, "Failed to load image!");
	// 	return 1;
	// }

	// return 1;
}

const luaL_Reg bsge_ui_methods[] = {
	{"load_font", lua_bsge_load_font},
	{"load_image", lua_bsge_load_image},

	{NULL, NULL},
};

int lua_bsge_connect_ui(BSGEWindow _window, lua_State* L) {
	luaL_newmetatable(L, "Ui");
	luaL_setfuncs(L, bsge_ui_methods, 0);

	lua_newtable(L);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "ui");

	return 0;
}

