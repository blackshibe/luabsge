#pragma once 

#include <lua.hpp>
#include "../../glad/glad.h"
#include "../../opengl/freetype.h"
#include "../luax.h"
#include <string>

struct bsgeImage;

int image_load(bsgeImage* texture, const char* path);
int image_download(bsgeImage* texture, const char* path);
void lua_bsge_init_image(sol::state &lua);

enum bsgeImageLoadSource {
	file,
	web
};

struct bsgeImage {
	int width;
	int height;
	int num_channels;
	GLuint id;

	bsgeImage(sol::this_state lua, const char* src, bsgeImageLoadSource file_source = bsgeImageLoadSource::file)  {
		if (file_source == bsgeImageLoadSource::file) {
			if (image_load(this, src) != 0) {
				luax_push_error(lua.lua_state(), "Failed to load image!");
			}
		} else {
			if (image_download(this, src) != 0) {
				luax_push_error(lua.lua_state(), "Failed to load image!");
			}	
		}

    }
};


