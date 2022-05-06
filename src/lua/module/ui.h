#pragma once

#include <lua.hpp>

#include "../../opengl/window.h"
#include "../../opengl/freetype.h"
#include "../luax.h"
#include "../class/font.h"

struct bsgeImage {
	int width;
	int height;
	int num_channels;
	GLuint id;
};

int lua_bsge_load_font(lua_State* L);
int lua_bsge_load_image(lua_State* L);
int lua_bsge_connect_ui(BSGEWindow _window, lua_State* L);
