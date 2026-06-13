#pragma once

#include "../../opengl/window.h"

glm::vec2 get_window_dimensions();
void set_vsync(bool enabled);
bool get_vsync();
void set_window_size(int width, int height);
void set_fullscreen(bool enabled);
void maximize_window();
void lua_bsge_connect_window(BSGEWindow *_window, sol::state &lua);