#include "object.h"

void lua_bsge_init_object(sol::state &lua) {
	lua.new_usertype<BSGEObject>("Object",
                                sol::constructors<BSGEObject()>()
    );
}
