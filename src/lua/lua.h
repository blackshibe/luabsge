#pragma once

#include <lua.hpp>
#include <chrono>

#include "../opengl/window.h"
#include "../include/colors.h"

#include "module/lua_rendering.h"
#include "module/lua_ui.h"
#include "module/lua_window.h"
#include "module/lua_glm_bindings.h"

#include "class/signal.h"
#include "class/vector3.h"
#include "class/vector2.h"
#include "class/color.h"
#include "class/mesh.h"
#include "class/textlabel.h"
#include "class/font.h"
#include "class/image.h"
#include "class/template.h"
#include "class/camera.h"

int64_t now();

// adds a prefix to Lua print
// lbaselib.c
int lua_base_print(lua_State* L);
int lua_warn(lua_State* L);
int lua_print(lua_State* L);
int lua_now(lua_State* L);

extern BSGEWindow* context_window;
int bsge_lua_init_state(BSGEWindow* _window, lua_State* L);