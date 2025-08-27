#include "mesh_tbo.h"

TextureBufferObject meshTriangles;
TextureBufferObject meshes;

struct  meshTBOTriangle {
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
};

// vec3s as vec4s because tbos are a pain in the ass
alignas(16) struct meshTBOMetadata {
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

void update_tbo(MeshBufferObject &self) {
    printf("meshTBOMetadata %i", sizeof(meshTBOMetadata));
    
    meshTBOMetadata metadata;
    metadata.matrix = self.matrix;
    metadata.inv_matrix = glm::inverse(self.matrix);
    metadata.color = glm::vec4(self.color, 1.0);
    metadata.emissive = self.emissive;
    metadata.triangles = self.triangles;
    metadata.box_min = mat4_to_vec4(glm::translate(self.matrix, self.box_min));
    metadata.box_max = mat4_to_vec4(glm::translate(self.matrix, self.box_max));

    upload_tbo_element(meshes, self.index, &metadata);
}

void lua_bsge_init_mesh_tbo(sol::state &lua) {
    meshTriangles = setup_tbo(GL_RGB32F, MESH_MAX_TRIANGLE_BUFFER_COUNT, sizeof(meshTBOTriangle));
    meshes = setup_tbo(GL_RGBA32F, MESH_MAX_TRIANGLE_BUFFER_COUNT, sizeof(meshTBOMetadata));

    lua.new_usertype<MeshBufferObject>("MeshBufferObject",
                                "register", [](MeshBufferObject &self, meshData mesh) {
                                    self.index = meshCount++;
                                    self.triangles = 0;

                                    glm::vec3 min_position = glm::vec3(10000.0f, 10000.0f, 10000.0f);
                                    glm::vec3 max_position = glm::vec3(-10000.0f, -10000.0f, -10000.0f);
                                    for (size_t i = 0; i < mesh.geometry.indices.size(); i += 3) {
                                        uint32_t idx0 = mesh.geometry.indices[i];
                                        uint32_t idx1 = mesh.geometry.indices[i + 1];
                                        uint32_t idx2 = mesh.geometry.indices[i + 2];

                                        meshTBOTriangle t;
                                        t.p1 = mesh.geometry.vertices[idx0].position;
                                        t.p2 = mesh.geometry.vertices[idx1].position;
                                        t.p3 = mesh.geometry.vertices[idx2].position;

                                        min_position = glm::vec3(
                                            std::min(std::min(std::min(min_position.x, t.p1.x), t.p2.x), t.p3.x),
                                            std::min(std::min(std::min(min_position.y, t.p1.y), t.p2.y), t.p3.y),
                                            std::min(std::min(std::min(min_position.z, t.p1.z), t.p2.z), t.p3.z)
                                        );

                                        max_position = glm::vec3(
                                            std::max(std::max(std::max(max_position.x, t.p1.x), t.p2.x), t.p3.x),
                                            std::max(std::max(std::max(max_position.y, t.p1.y), t.p2.y), t.p3.y),
                                            std::max(std::max(std::max(max_position.z, t.p1.z), t.p2.z), t.p3.z)
                                        );

                                        upload_tbo_element(meshTriangles, triangle_count, &t);

                                        self.triangles++;
                                        triangle_count++;
                                    }

                                    self.box_min = min_position;
                                    self.box_max = max_position;

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
                                "box_min", &MeshBufferObject::box_min,
                                "box_max", &MeshBufferObject::box_max,

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

