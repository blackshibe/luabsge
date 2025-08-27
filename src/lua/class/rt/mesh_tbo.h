#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include "../mesh.h"
#include "tbo.h"
#include <stdlib.h>
#include <algorithm>

#define MESH_MAX_TRIANGLE_BUFFER_COUNT 2000 // Temporary

struct MeshBufferObject {
    glm::mat4 matrix;
    glm::vec3 color;
    glm::vec3 box_min;
    glm::vec3 box_max;
    float emissive;
    float triangles;
    
    int index;
};

void lua_bsge_init_mesh_tbo(sol::state &lua);