#pragma once

#include "assimp/scene.h"
#include "engine/engine.h"
#include "scene/instance/instance.h"

class EngineInstance;

namespace Lua::instance {
	static entt::registry *registry = nullptr;

	struct Instance {
		entt::entity entity;

	  public:
		Instance(std::string name);
		Instance(entt::entity entity);
		Instance(aiNode *node);
	};

	void init(EngineInstance *engine, sol::state &lua);
}
