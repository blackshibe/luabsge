#pragma once 

#include <lua.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "../../glad/glad.h"
#include <glm/glm.hpp>
#include "../module/lua_ui.h"
#include <GLFW/glfw3.h>
#include "rt/mesh_tbo.h"

void lua_bsge_init_mesh(sol::state &lua);
