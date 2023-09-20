#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stbi_image.h"

int image_load(lua_State* L) {
	bsgeImage* texture = (bsgeImage*)lua_touserdata(L, 1);
	const char* path = lua_tostring(L, 2);

	printf("[image.cpp] load path: %s\n", path);

	texture->width = 0;
	texture->height = 0;
	texture->num_channels = 0;

	// stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(
		path,
		&texture->width,
		&texture->height,
		&texture->num_channels,
		STBI_rgb_alpha
	);

	if (data) {
		printf("[main.cpp] image loaded\n");
		printf("id: %i\n", texture->id);
		printf("width: %i\n", texture->width);
		printf("height: %i\n", texture->height);

		glGenTextures(1, &texture->id);
		glBindTexture(GL_TEXTURE_2D, texture->id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}
	else {
		luax_push_error(L, "Failed to load image!");
		return 1;
	}

	return 0;
}

int image_index(lua_State* L) {
	// this, index
	printf("[image.cpp] newindex\n");
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "load") == 0) {
		lua_pushcclosure(L, image_load, 0);
		return 1;
	}

	return 0;
}

int image_newindex(lua_State* L) {
	printf("[image.cpp] newindex\n");

	return 0;
}

const luaL_Reg bsge_lua_image_metatable_access[] = {
	{"__index", image_index},
	{"__newindex", image_newindex},

	{NULL, NULL},
};

int lua_bsge_instance_image(lua_State* L) {
	bsgeImage* texture = (bsgeImage*)lua_newuserdata(L, sizeof(bsgeImage));
	luaL_getmetatable(L, "Image");
	lua_setmetatable(L, -2);

	return 1;
}

void lua_bsge_init_image(lua_State* L) {
	luaL_newmetatable(L, "Image");
	luaL_setfuncs(L, bsge_lua_image_metatable_access, 0);

	// todo: replace with a universal constructor
	lua_register(L, "_temp_Image", lua_bsge_instance_image);
}
