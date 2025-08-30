#include "static_registry.h"

void lua_bsge_set_registry_reference(entt::registry *ref) {
    registry_reference = ref;
}

entt::registry* lua_bsge_get_registry() {
    return registry_reference;
}