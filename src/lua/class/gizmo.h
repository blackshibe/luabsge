#pragma once

#include <sol/sol.hpp>
#include <lua.hpp>
#include "../../glad/glad.h"
#include "../../opengl/freetype.h"
#include "../luax.h"
#include "../module/lua_ui.h"

struct GizmoVertex {
    alignas(4) glm::vec3 position;
    alignas(4) glm::vec3 color;
};

void lua_bsge_gizmo_begin_frame(glm::mat4 camera_projection, glm::mat4 camera_transform);  
void lua_bsge_gizmo_end_frame();
void draw_grid(float size, int divisions, const glm::vec3& color);
void draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color);
void lua_bsge_init_gizmo(sol::state &lua);