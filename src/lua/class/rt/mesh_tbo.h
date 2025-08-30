#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include "../mesh.h"
#include "tbo.h"
#include <stdlib.h>
#include <algorithm>
#include "../mesh.h"

#define MESH_MAX_TRIANGLE_BUFFER_COUNT 10000 // Temporary

struct MeshBufferObject {
    glm::mat4 matrix;
    glm::vec3 color;
    glm::vec3 box_min;
    glm::vec3 box_max;
    bsgeMesh mesh;

    float emissive;
    float triangles;
    int index;
};

void lua_bsge_init_mesh_tbo(sol::state &lua);