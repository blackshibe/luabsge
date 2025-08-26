#include "image.h"
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stbi_image.h"

int image_load(bsgeImage* texture, const char* path) {
	printf("[image.cpp] load path: %s\n", path);

	texture->width = 0;
	texture->height = 0;
	texture->num_channels = 0;

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
		return 0;
	}
	else {
		return 1;
	}
}

void lua_bsge_init_image(sol::state &lua) {
	lua_State *L = lua.lua_state();
	auto load = [&L](bsgeImage *image, const char *path) {
		if (image_load(image, path) != 0) {
			luax_push_error(L, "Failed to load image!");
			return 1;
		}
		return 0;
	};

	lua.new_usertype<bsgeImage>("Image",
							   "load", load);
}
