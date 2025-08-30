#include "object.h"

void lua_bsge_init_object(sol::state &lua) {
	lua.new_usertype<BSGEObject>("Object",
                                sol::constructors<BSGEObject()>(),
                                "add_component", [](sol::this_state state, BSGEObject &object, EcsComponentType type, const sol::table& data) {
                                    return lua_bsge_load_component(state, object, type, data);
                                },
                                "transform", [](BSGEObject &object, glm::mat4 transform) {
                                    entt::registry* registry = lua_bsge_get_registry();
    
                                    EcsObjectComponent& component = registry->get<EcsObjectComponent>(object.entity);
                                    component.transform = transform;
                                },
                                "parent", [](BSGEObject &object, BSGEObject& parent) {
                                    entt::registry* registry = lua_bsge_get_registry();
    
                                    EcsObjectComponent& component = registry->get<EcsObjectComponent>(object.entity);
                                    EcsObjectComponent& parent_component = registry->get<EcsObjectComponent>(parent.entity);
                                    component.parent = &parent_component;
                                }
    );

    lua["ECS_MESH_COMPONENT"] = ECS_MESH_COMPONENT;
	lua["ECS_MESH_TEXTURE_COMPONENT"] = ECS_MESH_TEXTURE_COMPONENT;
	lua["ECS_PHYSICS_COMPONENT"] = ECS_PHYSICS_COMPONENT;
}


void lua_bsge_load_component(sol::this_state lua, BSGEObject &object, EcsComponentType type, const sol::table& data) {
    entt::registry* registry = lua_bsge_get_registry();
          
    switch (type) {
        case EcsComponentType::ECS_MESH_COMPONENT: {
            // Extract mesh from Lua table
                sol::optional<bsgeMesh*> mesh_opt = data["mesh"];
                if (mesh_opt.has_value()) {
                    registry->emplace<EcsMeshComponent>(object.entity, mesh_opt.value());
                }
                break;
            }
            case EcsComponentType::ECS_MESH_TEXTURE_COMPONENT: {
                // Extract texture from Lua table
                sol::optional<bsgeImage*> texture_opt = data["texture"];
                if (texture_opt.has_value()) {
                    registry->emplace<EcsMeshTextureComponent>(object.entity, texture_opt.value());
                }
                break;
            }
            // case EcsComponentType::ECS_PHYSICS_COMPONENT: {
            //     // Extract is_dynamic from Lua table
            //     bool is_dynamic = data.get_or("is_dynamic", false);
            //     registry->emplace<PhysicsComponent>(entity, is_dynamic);
            //     break;
            // }
        }
}