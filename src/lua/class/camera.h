#pragma once

#include "../../opengl/freetype.h"
#include "../lua.h"
#include "../module/lua_window.h"
#include <lua.hpp>

// projection is calculated in the render step
struct BSGECameraMetadata {
	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));

	float fov = 70.0f;
	float near_clip = 0.1f;
	float far_clip = 100.0f;
};

glm::mat4 camera_get_projection_matrix(BSGECameraMetadata camera);
void lua_bsge_init_camera(sol::state &lua);