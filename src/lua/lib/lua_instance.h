#pragma once

#include "engine/engine.h"
#include "scene/instance/instance.h"

class EngineInstance;

namespace Lua::instance {
	static entt::registry *registry = nullptr;

	struct Instance {
		entt::entity entity;
		Instance(std::string name);
		Instance(entt::entity entity);
	};

	void init(EngineInstance &engine, sol::state &lua);
}
