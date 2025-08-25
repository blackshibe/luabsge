#pragma once

#include "../../opengl/window.h"

glm::vec2 get_window_dimensions();
void lua_bsge_connect_window(BSGEWindow *_window, sol::state &lua);