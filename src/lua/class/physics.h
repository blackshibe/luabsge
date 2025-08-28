#pragma once

#include "../lua.h"
#include <lua.hpp>
#include "../../physics/jolt.h"

void lua_bsge_init_physics(sol::state &lua);

struct PhysicsObject {
    JPH::BodyID id;

    PhysicsObject(meshData mesh, bool is_dynamic) {
        id = BSGE::Physics::create_body(mesh, is_dynamic);
    }

};