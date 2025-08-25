#include "lua_opengl_constants.h"

void lua_bsge_init_opengl_constants(sol::state &lua) {
    // Shader types
    lua["GL_VERTEX_SHADER"] = GL_VERTEX_SHADER;
    lua["GL_FRAGMENT_SHADER"] = GL_FRAGMENT_SHADER;
    lua["GL_GEOMETRY_SHADER"] = GL_GEOMETRY_SHADER;
    lua["GL_TESS_CONTROL_SHADER"] = GL_TESS_CONTROL_SHADER;
    lua["GL_TESS_EVALUATION_SHADER"] = GL_TESS_EVALUATION_SHADER;
    
    // Texture formats
    lua["GL_RGB"] = GL_RGB;
    lua["GL_RGBA"] = GL_RGBA;
    lua["GL_RED"] = GL_RED;
    lua["GL_RG"] = GL_RG;
    
    // Texture parameters
    lua["GL_TEXTURE_2D"] = GL_TEXTURE_2D;
    lua["GL_TEXTURE_MAG_FILTER"] = GL_TEXTURE_MAG_FILTER;
    lua["GL_TEXTURE_MIN_FILTER"] = GL_TEXTURE_MIN_FILTER;
    lua["GL_TEXTURE_WRAP_S"] = GL_TEXTURE_WRAP_S;
    lua["GL_TEXTURE_WRAP_T"] = GL_TEXTURE_WRAP_T;
    lua["GL_LINEAR"] = GL_LINEAR;
    lua["GL_NEAREST"] = GL_NEAREST;
    lua["GL_REPEAT"] = GL_REPEAT;
    lua["GL_CLAMP_TO_EDGE"] = GL_CLAMP_TO_EDGE;
    
    // Blend functions
    lua["GL_BLEND"] = GL_BLEND;
    lua["GL_SRC_ALPHA"] = GL_SRC_ALPHA;
    lua["GL_ONE_MINUS_SRC_ALPHA"] = GL_ONE_MINUS_SRC_ALPHA;
    
    // Depth testing
    lua["GL_DEPTH_TEST"] = GL_DEPTH_TEST;
    lua["GL_LESS"] = GL_LESS;
    lua["GL_LEQUAL"] = GL_LEQUAL;
    
    // Buffer types
    lua["GL_COLOR_BUFFER_BIT"] = GL_COLOR_BUFFER_BIT;
    lua["GL_DEPTH_BUFFER_BIT"] = GL_DEPTH_BUFFER_BIT;
    lua["GL_STENCIL_BUFFER_BIT"] = GL_STENCIL_BUFFER_BIT;
}