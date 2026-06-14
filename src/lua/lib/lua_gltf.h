#include "engine/engine.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Lua::gltf {
	static entt::registry *registry = nullptr;
	static EngineInstance *engine = nullptr;

	Lua::instance::Instance import_scene(const char *path);
	void init(EngineInstance *engine, sol::state &lua);
}