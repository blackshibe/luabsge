#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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