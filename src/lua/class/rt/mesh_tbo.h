#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include "../mesh.h"
#include "tbo.h"

#define MESH_MAX_TRIANGLE_BUFFER_COUNT 200 // Temporary

void lua_bsge_init_mesh_tbo(sol::state &lua);