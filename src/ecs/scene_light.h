#pragma once

#include <glm/glm.hpp>

namespace Ecs {
	struct DirectionalLightComponent {
		glm::vec3 direction;
		glm::vec3 color;

	  public:
		DirectionalLightComponent(glm::vec3 direction, glm::vec3 color) : direction(direction), color(color) {}
	};

	struct PointLightComponent {
		glm::vec3 position;
		glm::vec3 color;

	  public:
		PointLightComponent(glm::vec3 position, glm::vec3 color) : position(position), color(color) {}
	};

}
