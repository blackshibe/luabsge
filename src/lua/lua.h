#pragma once

#define SOL_ALL_SAFETIES_ON 1

#include <chrono>
#include <lua.hpp>
#include <sol/sol.hpp>

#include "../include/colors.h"
#include "../opengl/window.h"

#include "module/lua_glm_bindings.h"
#include "module/lua_imgui_bindings.h"
#include "module/lua_opengl_constants.h"
#include "module/lua_rendering.h"
#include "module/lua_ui.h"
#include "module/lua_window.h"

#include "class/camera.h"
#include "class/color.h"
#include "class/font.h"
#include "class/image.h"
#include "class/mesh.h"
#include "class/signal.h"
#include "class/template.h"
#include "class/textlabel.h"
#include "class/shader.h"
#include "class/vfx.h"
#include "class/input.h"
#include "class/gizmo.h"

#include "class/rt/tbo.h"
#include "class/rt/sphere_tbo.h"
#include "class/rt/mesh_tbo.h"

int64_t now();

// adds a prefix to Lua print
// lbaselib.c
int lua_base_print(lua_State *L);
int lua_warn(lua_State *L);
int lua_print(lua_State *L);
int lua_now(lua_State *L);

extern BSGEWindow *context_window;
int bsge_lua_init_state(BSGEWindow *window, sol::state &lua);