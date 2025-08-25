#pragma once 

#include <lua.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../glad/glad.h"
#include <glm/glm.hpp>
#include "../module/lua_ui.h"
#include <GLFW/glfw3.h>

struct meshVertexData {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct meshGeometry {
	std::vector<meshVertexData> vertices;
	std::vector<unsigned int> indices;
};

struct meshData {
	int indices_count;
	glm::mat4 matrix;
	meshGeometry geometry;

	unsigned int texture;
	unsigned int vbo;
	unsigned int vao;
	unsigned int ebo;
};

void lua_bsge_init_mesh(sol::state &lua);
