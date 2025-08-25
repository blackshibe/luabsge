#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>
#include "tbo.h"

#define SPHERE_MAX_COUNT 128

struct SphereTBO {
    glm::vec3 center;
    float radius;
    glm::vec3 color;
    float emissive;
};

void lua_bsge_init_sphere_tbo(sol::state &lua);
