#include "lua_opengl_constants.h"
#include <GLFW/glfw3.h>

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
    
    // GLFW Key codes
    lua["KEY_W"] = GLFW_KEY_W;
    lua["KEY_A"] = GLFW_KEY_A;
    lua["KEY_S"] = GLFW_KEY_S;
    lua["KEY_D"] = GLFW_KEY_D;
    lua["KEY_Q"] = GLFW_KEY_Q;
    lua["KEY_E"] = GLFW_KEY_E;
    lua["KEY_R"] = GLFW_KEY_R;
    lua["KEY_T"] = GLFW_KEY_T;
    lua["KEY_Y"] = GLFW_KEY_Y;
    lua["KEY_U"] = GLFW_KEY_U;
    lua["KEY_I"] = GLFW_KEY_I;
    lua["KEY_O"] = GLFW_KEY_O;
    lua["KEY_P"] = GLFW_KEY_P;
    lua["KEY_F"] = GLFW_KEY_F;
    lua["KEY_G"] = GLFW_KEY_G;
    lua["KEY_H"] = GLFW_KEY_H;
    lua["KEY_J"] = GLFW_KEY_J;
    lua["KEY_K"] = GLFW_KEY_K;
    lua["KEY_L"] = GLFW_KEY_L;
    lua["KEY_Z"] = GLFW_KEY_Z;
    lua["KEY_X"] = GLFW_KEY_X;
    lua["KEY_C"] = GLFW_KEY_C;
    lua["KEY_V"] = GLFW_KEY_V;
    lua["KEY_B"] = GLFW_KEY_B;
    lua["KEY_N"] = GLFW_KEY_N;
    lua["KEY_M"] = GLFW_KEY_M;
    
    lua["GL_TEXTURE0"] = GL_TEXTURE0;
    lua["GL_TEXTURE1"] = GL_TEXTURE1;
    lua["GL_TEXTURE2"] = GL_TEXTURE2;
    lua["GL_TEXTURE3"] = GL_TEXTURE3;
    lua["GL_TEXTURE4"] = GL_TEXTURE4;
    lua["GL_TEXTURE5"] = GL_TEXTURE5;

    // Numbers
    lua["KEY_0"] = GLFW_KEY_0;
    lua["KEY_1"] = GLFW_KEY_1;
    lua["KEY_2"] = GLFW_KEY_2;
    lua["KEY_3"] = GLFW_KEY_3;
    lua["KEY_4"] = GLFW_KEY_4;
    lua["KEY_5"] = GLFW_KEY_5;
    lua["KEY_6"] = GLFW_KEY_6;
    lua["KEY_7"] = GLFW_KEY_7;
    lua["KEY_8"] = GLFW_KEY_8;
    lua["KEY_9"] = GLFW_KEY_9;
    
    // Special keys
    lua["KEY_SPACE"] = GLFW_KEY_SPACE; // @ANNOTATE
    lua["KEY_ENTER"] = GLFW_KEY_ENTER;
    lua["KEY_TAB"] = GLFW_KEY_TAB;
    lua["KEY_BACKSPACE"] = GLFW_KEY_BACKSPACE;
    lua["KEY_DELETE"] = GLFW_KEY_DELETE;
    lua["KEY_LEFT"] = GLFW_KEY_LEFT;
    lua["KEY_RIGHT"] = GLFW_KEY_RIGHT;
    lua["KEY_UP"] = GLFW_KEY_UP;
    lua["KEY_DOWN"] = GLFW_KEY_DOWN;
    lua["KEY_PAGE_UP"] = GLFW_KEY_PAGE_UP;
    lua["KEY_PAGE_DOWN"] = GLFW_KEY_PAGE_DOWN;
    lua["KEY_HOME"] = GLFW_KEY_HOME;
    lua["KEY_END"] = GLFW_KEY_END;
    lua["KEY_INSERT"] = GLFW_KEY_INSERT;
    lua["KEY_LEFT_SHIFT"] = GLFW_KEY_LEFT_SHIFT;
    lua["KEY_RIGHT_SHIFT"] = GLFW_KEY_RIGHT_SHIFT;
    lua["KEY_LEFT_CONTROL"] = GLFW_KEY_LEFT_CONTROL;
    lua["KEY_RIGHT_CONTROL"] = GLFW_KEY_RIGHT_CONTROL;
    lua["KEY_LEFT_ALT"] = GLFW_KEY_LEFT_ALT;
    lua["KEY_RIGHT_ALT"] = GLFW_KEY_RIGHT_ALT;
    lua["KEY_ESCAPE"] = GLFW_KEY_ESCAPE;
    
    // Function keys
    lua["KEY_F1"] = GLFW_KEY_F1;
    lua["KEY_F2"] = GLFW_KEY_F2;
    lua["KEY_F3"] = GLFW_KEY_F3;
    lua["KEY_F4"] = GLFW_KEY_F4;
    lua["KEY_F5"] = GLFW_KEY_F5;
    lua["KEY_F6"] = GLFW_KEY_F6;
    lua["KEY_F7"] = GLFW_KEY_F7;
    lua["KEY_F8"] = GLFW_KEY_F8;
    lua["KEY_F9"] = GLFW_KEY_F9;
    lua["KEY_F10"] = GLFW_KEY_F10;
    lua["KEY_F11"] = GLFW_KEY_F11;
    lua["KEY_F12"] = GLFW_KEY_F12;
}