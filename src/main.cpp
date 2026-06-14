#include "engine/engine.h"
#include "rendering/window.h"
#include "util/output.h"

static Output output;

int main(int argc, char *argv[]) {

	output.info("running %s", LUA_VERSION);
	output.info("LuaBSGE %s", Engine::VERSION); 

	try {
		// engine instance first starts up the user context, which
		// will then configure properties for WindowInstance and any
		// assets the engine uses
		EngineInstance engine = EngineInstance();

		// create instance, connect to render loop
		engine.preflight();

		// takeoff
		engine.start();

		// BSGE::Physics::init();
	} catch (const std::exception& exception) {
		output.error("runtime error: %s", exception.what());
	}

	glfwTerminate();

	return 0;
}
