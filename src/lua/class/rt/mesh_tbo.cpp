#include "mesh_tbo.h"

TextureBufferObject meshTriangles;
TextureBufferObject meshes;

struct  meshTBOTriangle {
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
};

// vec3s as vec4s because tbos are a pain in the ass
struct meshTBOBufferData {
    glm::mat4 matrix; //64b
    glm::mat4 inv_matrix; //64b
    glm::vec4 color; // 16
    glm::vec4 box_min; // 16
    glm::vec4 box_max; // 16

    float triangles; // 4
    float emissive; // 4
    float padding[2]; // for 128 bit size
};

int meshCount = 0;
int triangle_count = 0;

struct boundingBox {
    glm::vec3 world_min;
    glm::vec3 world_max;
};

// AABB box positions become completely invalid once the mesh is rotated
// TODO: make a box model where a select amount of points is checked, rather than all of them
// TODO: pass mesh by pointer somehow
boundingBox get_bounding_box(MeshBufferObject &self) {
    boundingBox box;
    meshData mesh = self.mesh;
    
    glm::vec3 min_position = glm::vec3(10000.0f, 10000.0f, 10000.0f);
    glm::vec3 max_position = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
    for (size_t i = 0; i < mesh.geometry.indices.size(); i += 3) {
        glm::vec3 p1 = mat4_to_vec3(glm::translate(self.matrix, mesh.geometry.vertices[mesh.geometry.indices[i]].position));
        glm::vec3 p2 = mat4_to_vec3(glm::translate(self.matrix, mesh.geometry.vertices[mesh.geometry.indices[i + 1]].position));
        glm::vec3 p3 = mat4_to_vec3(glm::translate(self.matrix, mesh.geometry.vertices[mesh.geometry.indices[i + 2]].position));

        min_position = glm::vec3(
            std::min(std::min(std::min(min_position.x, p1.x), p2.x), p3.x),
            std::min(std::min(std::min(min_position.y, p1.y), p2.y), p3.y),
            std::min(std::min(std::min(min_position.z, p1.z), p2.z), p3.z)
        );

        max_position = glm::vec3(
            std::max(std::max(std::max(max_position.x, p1.x), p2.x), p3.x),
            std::max(std::max(std::max(max_position.y, p1.y), p2.y), p3.y),
            std::max(std::max(std::max(max_position.z, p1.z), p2.z), p3.z)
        );
    }

    box.world_min = min_position;
    box.world_max = max_position;

    return box;
}

// AABB 
void update_tbo(MeshBufferObject &self) {
    meshTBOBufferData metadata;
    metadata.matrix = self.matrix;
    metadata.inv_matrix = glm::inverse(self.matrix);
    metadata.color = glm::vec4(self.color, 1.0);
    metadata.emissive = self.emissive;
    metadata.triangles = self.triangles;

    boundingBox bb = get_bounding_box(self);
    metadata.box_max = glm::vec4(bb.world_max, 1.0);
    metadata.box_min = glm::vec4(bb.world_min, 1.0);

    upload_tbo_element(meshes, self.index, &metadata);
}

void lua_bsge_init_mesh_tbo(sol::state &lua) {
    meshTriangles = setup_tbo(GL_RGB32F, MESH_MAX_TRIANGLE_BUFFER_COUNT, sizeof(meshTBOTriangle));
    meshes = setup_tbo(GL_RGBA32F, MESH_MAX_TRIANGLE_BUFFER_COUNT, sizeof(meshTBOBufferData));

    lua.new_usertype<boundingBox>("boundingBox",
                                "world_min", &boundingBox::world_min,
                                "world_max", &boundingBox::world_max
                            );

    lua.new_usertype<MeshBufferObject>("MeshBufferObject",
                                "register", [](MeshBufferObject &self) {
                                    self.index = meshCount++;
                                    self.triangles = 0;

                                    meshData mesh = self.mesh;
                                    for (size_t i = 0; i < mesh.geometry.indices.size(); i += 3) {
                                        uint32_t idx0 = mesh.geometry.indices[i];
                                        uint32_t idx1 = mesh.geometry.indices[i + 1];
                                        uint32_t idx2 = mesh.geometry.indices[i + 2];

                                        meshTBOTriangle t;
                                        t.p1 = mesh.geometry.vertices[idx0].position;
                                        t.p2 = mesh.geometry.vertices[idx1].position;
                                        t.p3 = mesh.geometry.vertices[idx2].position;

                                        upload_tbo_element(meshTriangles, triangle_count, &t);

                                        self.triangles++;
                                        triangle_count++;
                                    }
                                    
                                    update_tbo(self);

                                    return self;
                                },

                                "update", [](MeshBufferObject &self) {
                                    update_tbo(self);
                                },

                                "matrix", &MeshBufferObject::matrix,
                                "color", &MeshBufferObject::color,
                                "emissive", &MeshBufferObject::emissive,
                                "triangles", &MeshBufferObject::triangles,
                                "mesh", &MeshBufferObject::mesh,
                    
                                "get_bounding_box", [](MeshBufferObject &self) {
                                    return get_bounding_box(self);
                                },

                                "get_count", []() {
                                    return meshCount;
                                },

                                "bind_textures", [](int triangle_texture_slot, int mesh_texture_slot) {
                                    glActiveTexture(triangle_texture_slot);
#if USE_EMSCRIPTEN
                                    // glBindTexture(GL_TEXTURE_2D, meshTriangles.tboTexture);
#else
                                    glBindTexture(GL_TEXTURE_BUFFER, meshTriangles.tboTexture);
#endif

                                    glActiveTexture(mesh_texture_slot);
#if USE_EMSCRIPTEN
                                    // glBindTexture(GL_TEXTURE_2D, meshes.tboTexture);
#else
                                    glBindTexture(GL_TEXTURE_BUFFER, meshes.tboTexture);
#endif
                                }
                            );

}

