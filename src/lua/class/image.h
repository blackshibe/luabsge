#pragma once 

#include <lua.hpp>
#include "../../glad/glad.h"
#include "../../opengl/freetype.h"
#include "../luax.h"
#include <string>

struct bsgeImage;

int image_load(bsgeImage* texture, const char* path);
void lua_bsge_init_image(sol::state &lua);

struct bsgeImage {
	int width;
	int height;
	int num_channels;
	GLuint id;

	bsgeImage(sol::this_state &lua, const char* src)  {
        if (image_load(this, src) != 0) {
            luax_push_error(lua.lua_state(), "Failed to load image!");
        }
    }
};


