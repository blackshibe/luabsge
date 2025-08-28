#pragma once

#include "../lua.h"
#include <lua.hpp>
#include "../../physics/jolt.h"

void lua_bsge_init_physics(sol::state &lua);

struct PhysicsObject {
    JPH::BodyID id;

    PhysicsObject(meshData mesh) {
        id = BSGE::Physics::create_body(mesh);
        printf("PhysicsObject created with id: %i\n", id);
    }

};