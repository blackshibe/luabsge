#include "../class/mesh.h"
#include "../class/image.h"
#include "../../physics/jolt.h"
#include <entt/entt.hpp>

struct EcsObjectComponent {
    glm::mat4 transform;
    entt::entity parent = entt::null;

    EcsObjectComponent() : transform(glm::mat4(1.0f)) {}
    EcsObjectComponent(glm::mat4 *transform, entt::entity* parent) : transform(*transform), parent(*parent) {}
};

// TODO: Refactor your mesh system to avoid storing OpenGL resources in components
struct EcsMeshComponent {
    bsgeMesh mesh;

    EcsMeshComponent(bsgeMesh* start_mesh) : mesh(*start_mesh) {}
};

struct EcsMeshTextureComponent {
    bsgeImage texture;

    EcsMeshTextureComponent(bsgeImage* start_texture) : texture(*start_texture) {}
};

struct EcsPhysicsComponent {
    JPH::BodyID body_id;
    bool is_dynamic;

    EcsPhysicsComponent(JPH::BodyID id, bool dynamic) : body_id(id), is_dynamic(dynamic) {}
};

// struct EcsTextlabelComponent {
//     bsgeTextlabel textlabel;

//     EcsPhysicsComponent(bsgeTextlabel* text) : textlabel(*text) {}
// };

// struct EcsFrameComponent {
//     bsgeTextlabel textlabel;

//     EcsPhysicsComponent(bsgeTextlabel* text) : textlabel(*text) {}
// };