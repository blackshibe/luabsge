#pragma once 

#include <lua.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "../../glad/glad.h"
#include "../luax.h"
#include "string"
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sol/sol.hpp>

struct bsgeMesh;

int mesh_load(bsgeMesh *bsgemesh, const char *path);
void lua_bsge_init_mesh(sol::state &lua);

struct meshVertexData {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct meshGeometry {
	std::vector<meshVertexData> vertices;
	std::vector<unsigned int> indices;
};

struct bsgeMesh {
	int indices_count;
	glm::mat4 matrix;
	meshGeometry geometry;

	unsigned int texture;
	unsigned int vbo;
	unsigned int vao;
	unsigned int ebo;

	bsgeMesh(sol::this_state &lua, const char* src)  {
		if (mesh_load(this, src) != 0) {
			luax_push_error(lua.lua_state(), "Failed to load mesh!");
		}
	}
};