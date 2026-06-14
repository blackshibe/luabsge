#pragma once

#include <lua.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sol/sol.hpp>

struct bsgeMesh;

int mesh_load(bsgeMesh *bsgemesh, const char *path);
int mesh_render(sol::state &lua, glm::mat4 matrix, bsgeMesh &bsgemesh);
void lua_bsge_init_mesh(sol::state &lua);

struct bsgeMesh {
	glm::mat4 matrix;
	glm::vec4 color;

	bsgeMesh(sol::this_state lua, const char *src) : matrix(glm::mat4(1)), color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) {
		if (mesh_load(this, src) != 0) {
			luax_push_error(lua.lua_state(), "Failed to load mesh!");
		}
	}
};