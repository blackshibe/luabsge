#include "sphere_tbo.h"

int sphere_count;
SphereTBO tbo_buffer[SPHERE_MAX_COUNT];
TextureBufferObject spheres;

void make_sphere(glm::vec3 position, glm::vec3 color, float radius, float emissive) {
    SphereTBO tbo = SphereTBO();
    tbo.color = color;
    tbo.center = position;
    tbo.emissive = emissive;
    tbo.radius = radius;

    tbo_buffer[sphere_count] = tbo;
    upload_tbo_element(spheres, sphere_count, &tbo);

    sphere_count += 1;
}

void lua_bsge_init_sphere_tbo(sol::state &lua) {
    for (int i = 0; i < SPHERE_MAX_COUNT; i++) {
        tbo_buffer[i] = SphereTBO();
    }

    spheres = setup_tbo(GL_RGBA32F, SPHERE_MAX_COUNT, sizeof(SphereTBO), tbo_buffer);

	lua.set_function("TEMP_make_sphere", make_sphere);
	lua.set_function("TEMP_get_tbo_texture", []() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, spheres.tboTexture);
        return spheres.tboTexture;
    });
    lua.set_function("TEMP_get_tbo_texture_count", []() {
        return sphere_count;
    });
}
