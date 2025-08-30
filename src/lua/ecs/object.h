#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include "static_registry.h"

struct BSGEObject {
    entt::entity entity;
    BSGEObject* parent;
    glm::mat4 transform;

    BSGEObject() {
        this->entity = lua_bsge_get_registry()->create();
        this->parent = nullptr;
        this->transform = glm::mat4(1);
    }
};

void lua_bsge_init_object(sol::state &lua);