#include "../class/mesh.h"
#include "../class/image.h"

struct EcsObjectComponent {
    glm::mat4 transform;
    EcsObjectComponent* parent;

    EcsObjectComponent() : transform(glm::mat4(1.0f)), parent(nullptr) {}
    EcsObjectComponent(glm::mat4 *transform, EcsObjectComponent* parent) : transform(*transform), parent(parent) {}
};

struct EcsMeshComponent {
    bsgeMesh mesh;

    EcsMeshComponent(bsgeMesh* start_mesh) : mesh(*start_mesh) {}
};

struct EcsMeshTextureComponent {
    bsgeImage texture;

    EcsMeshTextureComponent(bsgeImage* start_texture) : texture(*start_texture) {}
};

// TODO
struct EcsPhysicsObjectComponent {

    EcsPhysicsObjectComponent() {}
};