#include "template.h"

struct TemplateStruct {
	int function_calls;
};

void lua_bsge_init_template(sol::state &lua) {
	lua.new_usertype<TemplateStruct>("Template",
									 "function_calls", &TemplateStruct::function_calls);
}
