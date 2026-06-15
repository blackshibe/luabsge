#pragma once

#include "assimp/scene.h"
#include "ecs/instance.h"
#include "engine/engine.h"
#include "glm/ext/matrix_float4x4.hpp"


class EngineInstance;

namespace Lua::instance {
	static entt::registry *registry = nullptr;

	struct Instance {
		entt::entity entity;

	  public:
		Instance(std::string name);
		Instance(entt::entity entity);
		Instance(aiNode *node);

		void set_parent(Instance instance);
		void set_transform(glm::mat4 transform);
		void set_transform(const aiMatrix4x4 &transform);
	};

	void init(EngineInstance *engine, sol::state &lua);
}
