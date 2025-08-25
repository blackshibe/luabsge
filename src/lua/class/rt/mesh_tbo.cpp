#include "mesh_tbo.h"

TextureBufferObject meshTriangles;
TextureBufferObject meshes;

struct  meshTBOTriangle {
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
};

struct meshTBOMetadata {
    glm::mat4 matrix;
    float triangles;
};

int meshCount = 0;
int triangle_count = 0;

void lua_bsge_init_mesh_tbo(sol::state &lua) {
    meshTriangles = setup_tbo(GL_RGB32F, MESH_MAX_TRIANGLE_BUFFER_COUNT, sizeof(meshTBOTriangle));
    meshes = setup_tbo(GL_RGBA32F, MESH_MAX_TRIANGLE_BUFFER_COUNT, sizeof(meshTBOMetadata));

    lua.set_function("mesh_tbo_register_mesh", [](meshData mesh) {
        int tri_count = 0;
        std::vector<meshTBOTriangle> triangleData;

        for (size_t i = 0; i < mesh.geometry.indices.size(); i += 3) {
            uint32_t idx0 = mesh.geometry.indices[i];
            uint32_t idx1 = mesh.geometry.indices[i + 1];
            uint32_t idx2 = mesh.geometry.indices[i + 2];

            // Fetch the vertices and create a triangle
            const meshVertexData& v0 = mesh.geometry.vertices[idx0];
            const meshVertexData& v1 = mesh.geometry.vertices[idx1];
            const meshVertexData& v2 = mesh.geometry.vertices[idx2];

            meshTBOTriangle t;
            t.p1 = v0.position;
            t.p2 = v1.position;
            t.p3 = v2.position;

            upload_tbo_element(meshTriangles, triangle_count, &t);

            tri_count++;
            triangle_count++;
        }

        meshTBOMetadata metadata;
        metadata.matrix = mesh.matrix;
        metadata.triangles = tri_count;

        printf("value set to %i\n", triangle_count);
        printf("value set to %i\n", meshCount);

        upload_tbo_element(meshes, meshCount, &metadata);
        meshCount += 1;

        printf("meshTBOTriangle %i\n", sizeof(meshTBOTriangle));
        printf("meshTBOMetadata %i\n", sizeof(meshTBOMetadata));
    });

    lua.set_function("mesh_tbo_get_count", []() {
        return meshCount;
    });
                 
    lua.set_function("mesh_tbo_bind_meshTriangles", []() {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, meshTriangles.tboTexture);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_BUFFER, meshes.tboTexture);
    });

}

