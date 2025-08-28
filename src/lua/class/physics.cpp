#include "physics.h"

glm::mat4x4 lua_get_body_transform(PhysicsObject& object) {
    return BSGE::Physics::get_body_transform(object.id);
}

void lua_bsge_init_physics(sol::state &lua) {
	lua.new_usertype<PhysicsObject>("PhysicsObject",
                        sol::constructors<PhysicsObject(meshData)>(),
                        "get_transform", lua_get_body_transform
								);
}
