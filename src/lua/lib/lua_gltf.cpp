#include "lua_gltf.h"
#include "util/output.h"

static Output output;

namespace Lua::gltf {

	void import_scene(const char *path) {
	}

	void init(EngineInstance &engine, sol::state &lua) {
		auto gltf_namespace = lua["GLTF"].get_or_create<sol::table>();

		gltf_namespace["import"] = [](std::string path) {
			output.info("importing %s", path.c_str());
		};
	};
}