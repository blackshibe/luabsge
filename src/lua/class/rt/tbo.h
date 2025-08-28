#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>

// TBOs are supported since 3.3 unlike SSBOs, but probably unpreferrable for quickly updating objects
// not that this matters in a ray tracer scenario
struct TextureBufferObject {
    GLuint tboBuffer;
    GLuint tboTexture;
    uint16_t item_size;
};

TextureBufferObject setup_tbo(int packing_size, uint16_t max_size, uint16_t obj_size) ;
void upload_tbo_element(TextureBufferObject obj, uint16_t index, const void* item);