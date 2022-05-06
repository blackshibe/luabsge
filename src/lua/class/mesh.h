#pragma once 

#include <lua.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../glad/glad.h"
#include <glm/glm.hpp>
#include "../module/ui.h"
#include <GLFW/glfw3.h>

struct meshVertexData {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct meshData {
	int indices_count;

	unsigned int texture;
	unsigned int vbo;
	unsigned int vao;
	unsigned int ebo;
};

int lua_bsge_new_mesh(lua_State* L);
void lua_bsge_init_mesh(lua_State* L);
