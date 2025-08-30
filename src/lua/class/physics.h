#pragma once

#include <lua.hpp>
#include "mesh.h"
#include "../../physics/jolt.h"

void lua_bsge_init_physics(sol::state &lua);

struct PhysicsObject {
    JPH::BodyID id;

    PhysicsObject(bsgeMesh mesh, bool is_dynamic) {
        id = BSGE::Physics::create_body(mesh, is_dynamic);
    }

};