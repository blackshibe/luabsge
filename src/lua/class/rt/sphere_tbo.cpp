#include "sphere_tbo.h"

int sphere_count;
TextureBufferObject spheres;

void make_sphere(glm::vec3 position, glm::vec3 color, float radius, float emissive) {
    SphereTBO tbo = SphereTBO();
    tbo.color = color;
    tbo.center = position;
    tbo.emissive = emissive;
    tbo.radius = radius;

    upload_tbo_element(spheres, sphere_count, &tbo);

    sphere_count += 1;
}

void lua_bsge_init_sphere_tbo(sol::state &lua) {
    spheres = setup_tbo(GL_RGBA32F, SPHERE_MAX_COUNT, sizeof(SphereTBO));

	lua.set_function("TEMP_make_sphere", make_sphere);
	lua.set_function("TEMP_bind_tbo_texture", []() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, spheres.tboTexture);
    });
    
    lua.set_function("TEMP_get_tbo_texture_count", []() {
        return sphere_count;
    });

    lua.new_usertype<SphereBufferObject>("SphereBufferObject",
                                "register", [](SphereBufferObject &self) {
                                    self.index = sphere_count++;

                                    SphereTBO tbo = SphereTBO();
                                    tbo.color = self.color;
                                    tbo.center = self.center;
                                    tbo.emissive = self.emissive;
                                    tbo.radius = self.radius;

                                    upload_tbo_element(spheres, self.index, &tbo);

                                    return self;
                                },

                                "update", []( SphereBufferObject &tbo) {
                                    return upload_tbo_element(spheres, tbo.index, &tbo);
                                },

                                "center", &SphereBufferObject::center,
                                "color", &SphereBufferObject::color,
                                "radius", &SphereBufferObject::radius,
                                "emissive", &SphereBufferObject::emissive,

                                "get_count", []() {
                                    return sphere_count;
                                },

                                "get_texture", []() {
                                    glActiveTexture(GL_TEXTURE0);
                                    glBindTexture(GL_TEXTURE_BUFFER, spheres.tboTexture);
                                    return spheres.tboTexture;
                                }
                            );
}


