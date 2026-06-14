#include "lua/luax.h"

static Output output;

namespace Lua::util {
	bool run_script(sol::state &lua, const char *filename) {
		output.info("run_script running %s", filename);

		sol::protected_function_result result = lua.safe_script_file(filename, &sol::script_pass_on_error);

		if (!result.valid()) {
			sol::error err = result;
			output.error("%s", err.what());
			return false;
		}

		return true;
	}
}
