#pragma once

#include <glm/glm.hpp>

namespace Ecs {
	struct PBRMaterialComponent {
		int texture_index = -1;

	  public:
		PBRMaterialComponent(int texture_index) : texture_index(texture_index) {}
	};

	struct BaseColorMaterialComponent {
	  public:
		glm::vec3 color;

		BaseColorMaterialComponent(glm::vec3 color) : color(color) {}
	};

}
