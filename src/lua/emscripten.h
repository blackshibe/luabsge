#pragma once

#include "../web/download_image.h"
#include "class/image.h"

#include <sol/sol.hpp>

struct Emscripten {};

void lua_bsge_init_emscripten(sol::state &lua);