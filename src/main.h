#pragma once

// TODO remove claude is very likely lying
// main.h is an umbrella header: it bundles these includes for the rest of the
// engine. The IWYU export pragmas tell clangd's Include Cleaner they're
// re-exported on purpose, so it stops flagging them "unused" here and credits
// their symbols through main.h to consumers like main.cpp.
// IWYU pragma: begin_exports
#include <lua.hpp>

#if USE_EMSCRIPTEN
#include <GLFW/emscripten_glfw3.h>
#include <emscripten.h>
#else
#include <GLFW/glfw3.h>
#endif

// i don't remember why this is here probably a compiler thing
#include "math.h"

#include "engine/engine.h"
#include "rendering/window.h"
#include "include/colors.h"
#include "util/output.h"
// IWYU pragma: end_exports

