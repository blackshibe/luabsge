#include "entt/entity/fwd.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "resource/mesh/mesh.h"
#include <glm/glm.hpp>
#include <sol/sol.hpp>

#include <entt/entt.hpp>
#include <string>

namespace Scene::ecs {
	enum EcsComponentType {
		INSTANCE,
	};

	struct Instance {
		std::string name = "Instance";
		glm::mat4 transform = glm::mat4(1);
		entt::entity parent = entt::null;
	};

	struct Mesh {
		MeshGeometry geometry;
	};

}
