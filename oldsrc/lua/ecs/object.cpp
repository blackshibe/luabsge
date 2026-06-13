#include "object.h"

void lua_bsge_init_object(sol::state &lua) {
	lua.new_usertype<BSGEObject>("Object",
                                sol::constructors<BSGEObject()>(),
                                "add_component", [](sol::this_state state, BSGEObject &object, EcsComponentType type, const sol::table& data) {
                                    return lua_bsge_load_component(state, object, type, data);
                                },
                                "get_component", [](sol::this_state state, BSGEObject &object, EcsComponentType type) {
                                    return lua_bsge_get_component(state, object, type);
                                },
                                "patch_component", [](sol::this_state state, BSGEObject &object, EcsComponentType type, const sol::table& data) {
                                    lua_bsge_patch_component(state, object, type, data);
                                },
                                "transform", sol::property(
                                    [](BSGEObject &object) -> glm::mat4 {
                                        entt::registry* registry = lua_bsge_get_registry();
                                        
                                        if (registry->all_of<EcsPhysicsComponent>(object.entity)) {
                                            auto& physics_comp = registry->get<EcsPhysicsComponent>(object.entity);
                                            return BSGE::Physics::get_body_transform(physics_comp.body_id);
                                        }
                                        
                                        auto& obj_comp = registry->get<EcsObjectComponent>(object.entity);
                                        return obj_comp.transform;
                                    },
                                    [](BSGEObject &object, glm::mat4 transform) {
                                        entt::registry* registry = lua_bsge_get_registry();
        
                                        registry->patch<EcsObjectComponent>(object.entity, [&transform](auto& comp) {
                                            comp.transform = transform;
                                        });
                                        
                                        if (registry->all_of<EcsPhysicsComponent>(object.entity)) {
                                            auto& physics_comp = registry->get<EcsPhysicsComponent>(object.entity);
                                            BSGE::Physics::set_body_transform(physics_comp.body_id, transform);
                                        }
                                    }),
                                "parent", sol::property([](BSGEObject &object, BSGEObject& parent) {
                                    entt::registry* registry = lua_bsge_get_registry();
    
                                    registry->patch<EcsObjectComponent>(object.entity, [&parent](auto& comp) {
                                        comp.parent = parent.entity;
                                    });
                                })
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
                sol::optional<glm::vec4> color_opt = data["color"];

                if (mesh_opt.value() == nullptr) {
                    luaL_error(lua, "ECS_MESH_COMPONENT requires a 'mesh' field");
                    return;
                } 

                bsgeMesh mesh = *mesh_opt.value();
                if (color_opt.has_value()) mesh.color = color_opt.value();

                registry->emplace<EcsMeshComponent>(object.entity, &mesh);

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
            case EcsComponentType::ECS_PHYSICS_COMPONENT: {
                // Extract mesh and is_dynamic from Lua table
                sol::optional<bsgeMesh*> mesh_opt = data["mesh"];
                bool is_dynamic = data.get_or("is_dynamic", false);
                if (mesh_opt.has_value()) {
                    // Get the object's current transform to initialize physics body
                    auto& obj_comp = registry->get<EcsObjectComponent>(object.entity);
                    
                    // Temporarily set mesh matrix to object transform for physics body creation
                    glm::mat4 original_mesh_matrix = mesh_opt.value()->matrix;
                    mesh_opt.value()->matrix = obj_comp.transform;
                    
                    JPH::BodyID body_id = BSGE::Physics::create_body(*mesh_opt.value(), is_dynamic);
                    
                    // Restore original mesh matrix
                    mesh_opt.value()->matrix = original_mesh_matrix;
                    
                    registry->emplace<EcsPhysicsComponent>(object.entity, body_id, is_dynamic);
                }
                break;
            }
        }
}

sol::object lua_bsge_get_component(sol::this_state lua, BSGEObject &object, EcsComponentType type) {
    entt::registry* registry = lua_bsge_get_registry();
    sol::state_view lua_state(lua);
    
    switch (type) {
        case EcsComponentType::ECS_MESH_COMPONENT: {
            if (registry->all_of<EcsMeshComponent>(object.entity)) {
                auto& mesh_comp = registry->get<EcsMeshComponent>(object.entity);
                sol::table result = lua_state.create_table();
                result["mesh"] = &mesh_comp.mesh;
                return result;
            }
            break;
        }
        case EcsComponentType::ECS_MESH_TEXTURE_COMPONENT: {
            if (registry->all_of<EcsMeshTextureComponent>(object.entity)) {
                auto& texture_comp = registry->get<EcsMeshTextureComponent>(object.entity);
                sol::table result = lua_state.create_table();
                result["texture"] = &texture_comp.texture;
                return result;
            }
            break;
        }
        case EcsComponentType::ECS_PHYSICS_COMPONENT: {
            if (registry->all_of<EcsPhysicsComponent>(object.entity)) {
                auto& physics_comp = registry->get<EcsPhysicsComponent>(object.entity);
                sol::table result = lua_state.create_table();
                result["body_id"] = physics_comp.body_id.GetIndexAndSequenceNumber();
                result["is_dynamic"] = physics_comp.is_dynamic;
                result["get_transform"] = [&physics_comp]() -> glm::mat4 {
                    return BSGE::Physics::get_body_transform(physics_comp.body_id);
                };
                return result;
            }
            break;
        }
    }
    
    return sol::nil;
}

void lua_bsge_patch_component(sol::this_state lua, BSGEObject &object, EcsComponentType type, const sol::table& data) {
    entt::registry* registry = lua_bsge_get_registry();
    
    switch (type) {
        case EcsComponentType::ECS_MESH_COMPONENT: {
            if (registry->all_of<EcsMeshComponent>(object.entity)) {
                registry->patch<EcsMeshComponent>(object.entity, [&data](auto& comp) {
                    sol::optional<glm::vec4*> color_opt = data["color"];
                    if (color_opt.has_value()) {
                        comp.mesh.color = *color_opt.value();
                    }
                });
            }
            break;
        }
        case EcsComponentType::ECS_MESH_TEXTURE_COMPONENT: {
            if (registry->all_of<EcsMeshTextureComponent>(object.entity)) {
                registry->patch<EcsMeshTextureComponent>(object.entity, [&data](auto& comp) {
                    sol::optional<bsgeImage*> texture_opt = data["texture"];
                    if (texture_opt.has_value()) {
                        comp.texture = *texture_opt.value();
                    }
                });
            }
            break;
        }
        case EcsComponentType::ECS_PHYSICS_COMPONENT: {
            // Physics components generally shouldn't be patched at runtime
            // as they require physics world updates
            break;
        }
    }
}