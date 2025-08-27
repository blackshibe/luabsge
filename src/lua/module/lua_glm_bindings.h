#pragma once

#include "../lua.h"
#include <lua.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <lua.hpp>

glm::vec4 mat4_to_vec4(glm::mat4 m);
glm::vec3 mat4_to_vec3(glm::mat4 m);
int lua_bsge_init_glm_bindings(sol::state &lua);