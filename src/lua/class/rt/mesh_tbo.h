#pragma once

#include "../../lua.h"
#include <lua.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include "../mesh.h"
#include "tbo.h"
#include <stdlib.h>
#include <algorithm>

#define MESH_MAX_TRIANGLE_BUFFER_COUNT 10000 // Temporary

// TODO circular dependency shitshow
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

struct MeshBufferObject {
    glm::mat4 matrix;
    glm::vec3 color;
    glm::vec3 box_min;
    glm::vec3 box_max;
    meshData mesh;

    float emissive;
    float triangles;
    int index;
};

void lua_bsge_init_mesh_tbo(sol::state &lua);