#include "lua_glm_bindings.h"

glm::vec2 make_vec2(float x, float y) {
	return glm::vec2(x, y);
}

glm::vec3 make_vec3(float x, float y, float z) {
	return glm::vec3(x, y, z);
}

glm::mat4 make_mat4() {
	return glm::mat4();
}

int lua_bsge_init_glm_bindings(sol::state &lua) {

	lua.new_usertype<glm::vec2>("Vec2",
								sol::meta_function::construct, make_vec2,
								"x", &glm::vec2::x,
								"y", &glm::vec2::y);

	lua.new_usertype<glm::vec3>("Vec3",
								sol::meta_function::construct, make_vec3,
								"x", &glm::vec3::x,
								"y", &glm::vec3::y,
								"z", &glm::vec3::z);

	lua.new_usertype<glm::mat4>("Mat4",
								sol::meta_function::construct, make_mat4);

	return 0;
}