
#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// We need a vertex format, so lets use this one. when creating a vertex format its very important to compact
// the data as much as possible, but for the current stage of the tutorial it wont matter.
// We will optimize this vertex format later. The reason the uv parameters are interleaved is due to alignement
// limitations on GPUs. We want this structure to match the shader version so interleaving it like this improves it.
struct MeshVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

struct MeshGeometry {
	std::string name;

	std::vector<MeshVertex> vertices;
	std::vector<unsigned int> indices;

  public:
	MeshGeometry(std::string name) : name(name) {}
};
