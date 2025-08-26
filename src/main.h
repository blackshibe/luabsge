#pragma once


#include "glad/glad.h"
#include <cstdio>
#include <iostream>
#include <lua.hpp>

#include "math.h"
#include "unistd.h"

#include "lua/lua.h"
#include "lua/luax.h"
#include "opengl/freetype.h"
#include "opengl/window.h"

#if USE_EMSCRIPTEN
#include <GLFW/emscripten_glfw3.h>
#include <emscripten.h>
#else
#include <GLFW/glfw3.h>
#endif
