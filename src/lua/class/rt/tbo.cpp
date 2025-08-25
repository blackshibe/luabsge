#include "tbo.h"

ShaderSSBOData spheres;

// TBOs are read only as far as i can tell(?)
// they are supported since 3.3 unlike SSBOs, but probably unpreferrable for quickly updating objects

GLuint tboBuffer;
GLuint tboTexture;

void updateSingleSphere(size_t index, const SphereTBO& sphere) {
    glBindBuffer(GL_TEXTURE_BUFFER, tboBuffer);
    glBufferSubData(GL_TEXTURE_BUFFER,
                   index * sizeof(SphereTBO),
                   sizeof(SphereTBO),
                   &sphere);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void make_sphere(glm::vec3 position, glm::vec3 color, float radius, bool emissive) {
    SphereTBO tbo = SphereTBO();
    tbo.color = color;
    tbo.center = position;
    tbo.emissive = emissive;
    tbo.radius = radius;

    spheres.TBO[spheres.amount] = tbo;
    updateSingleSphere(spheres.amount, tbo);

    spheres.amount += 1;
}

void lua_bsge_init_sphere_tbo(sol::state &lua) {
    spheres = ShaderSSBOData();
    spheres.amount = 0;
    for (int i = 0; i < SPHERE_MAX_COUNT; i++) {
        spheres.TBO[i] = SphereTBO();
    }

    glGenBuffers(1, &tboBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, tboBuffer);
    glBufferData(GL_TEXTURE_BUFFER, SPHERE_MAX_COUNT * sizeof(SphereTBO), spheres.TBO, GL_DYNAMIC_DRAW);

    glGenTextures(1, &tboTexture);
    glBindTexture(GL_TEXTURE_BUFFER, tboTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, tboBuffer);

	lua.set_function("TEMP_make_sphere", make_sphere);
	lua.set_function("TEMP_get_tbo_texture", []() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, tboTexture);
        return tboTexture;
    });
    lua.set_function("TEMP_get_tbo_texture_count", []() {
        return spheres.amount;
    });
}
