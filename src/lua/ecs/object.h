#include <sol/sol.hpp>
#include <glm/glm.hpp>

#include "../class/mesh.h"

#include "static_registry.h"
#include "mesh_component.h"

enum EcsComponentType  {
    ECS_MESH_COMPONENT,
    ECS_MESH_TEXTURE_COMPONENT,
    ECS_PHYSICS_COMPONENT
};

struct BSGEObject {
    entt::entity entity;

    BSGEObject() {
        entt::registry* registry = lua_bsge_get_registry();
        this->entity = lua_bsge_get_registry()->create();
        registry->emplace<EcsObjectComponent>(this->entity);
    }
};

void lua_bsge_init_object(sol::state &lua);
void lua_bsge_load_component(sol::this_state lua, BSGEObject &object, EcsComponentType type, const sol::table& data);
sol::object lua_bsge_get_component(sol::this_state lua, BSGEObject &object, EcsComponentType type);