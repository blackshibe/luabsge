#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>

// TBOs are supported since 3.3 unlike SSBOs, but probably unpreferrable for quickly updating objects
// not that this matters in a ray tracer scenario
struct TextureBufferObject {
    GLuint tboBuffer;
    GLuint tboTexture;
    uint item_size;
};

TextureBufferObject setup_tbo(int packing_size, uint max_size, uint obj_size) ;
void upload_tbo_element(TextureBufferObject obj, uint index, const void* item);