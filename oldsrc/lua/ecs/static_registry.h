#pragma once

#include <entt/entt.hpp>

static entt::registry* registry_reference = nullptr;

void lua_bsge_set_registry_reference(entt::registry *ref);
entt::registry* lua_bsge_get_registry();