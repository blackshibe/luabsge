#include "lua_glm_bindings.h"
#include <string>

glm::vec2 make_vec2(float x, float y) {
	return glm::vec2(x, y);
}

glm::vec2 make_vec2_single(float value) {
	return glm::vec2(value);
}

glm::vec3 make_vec3(float x, float y, float z) {
	return glm::vec3(x, y, z);
}

glm::vec3 make_vec3_single(float value) {
	return glm::vec3(value);
}

glm::mat4 make_mat4() {
	return glm::mat4();
}

int lua_bsge_init_glm_bindings(sol::state &lua) {

	lua.new_usertype<glm::vec2>("Vec2",
								sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(),
								"x", &glm::vec2::x,
								"y", &glm::vec2::y,
								
								// Arithmetic operations
								sol::meta_function::addition, [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
								sol::meta_function::subtraction, [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
								sol::meta_function::multiplication, sol::overload(
									[](const glm::vec2& a, const glm::vec2& b) { return a * b; },
									[](const glm::vec2& a, float b) { return a * b; },
									[](float a, const glm::vec2& b) { return a * b; }
								),
								sol::meta_function::division, sol::overload(
									[](const glm::vec2& a, const glm::vec2& b) { return a / b; },
									[](const glm::vec2& a, float b) { return a / b; }
								),
								sol::meta_function::unary_minus, [](const glm::vec2& v) { return -v; },
								
								// Vector operations
								"length", [](const glm::vec2& v) { return glm::length(v); },
								"normalize", [](const glm::vec2& v) { return glm::normalize(v); },
								"dot", [](const glm::vec2& a, const glm::vec2& b) { return glm::dot(a, b); },
								"distance", [](const glm::vec2& a, const glm::vec2& b) { return glm::distance(a, b); },
								
								// String representation
								sol::meta_function::to_string, [](const glm::vec2& v) {
									return "Vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
								});

	lua.new_usertype<glm::vec3>("Vec3",
								sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),
								"x", &glm::vec3::x,
								"y", &glm::vec3::y,
								"z", &glm::vec3::z,
								
								// Arithmetic operations
								sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
								sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
								sol::meta_function::multiplication, sol::overload(
									[](const glm::vec3& a, const glm::vec3& b) { return a * b; },
									[](const glm::vec3& a, float b) { return a * b; },
									[](float a, const glm::vec3& b) { return a * b; }
								),
								sol::meta_function::division, sol::overload(
									[](const glm::vec3& a, const glm::vec3& b) { return a / b; },
									[](const glm::vec3& a, float b) { return a / b; }
								),
								sol::meta_function::unary_minus, [](const glm::vec3& v) { return -v; },
								
								// Vector operations
								"length", [](const glm::vec3& v) { return glm::length(v); },
								"normalize", [](const glm::vec3& v) { return glm::normalize(v); },
								"dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); },
								"cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); },
								"distance", [](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); },
								
								// String representation
								sol::meta_function::to_string, [](const glm::vec3& v) {
									return "Vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
								});

	lua.new_usertype<glm::mat4>("Mat4",
								sol::constructors<glm::mat4(), glm::mat4(float)>(),
								
								// Matrix operations
								"translate", [](const glm::mat4& m, const glm::vec3& v) { 
									return glm::translate(m, v); 
								},

								"inverse", [](const glm::mat4& m) { 
									return glm::inverse(m); 
								},
								
								"rotate", [](const glm::mat4& m, float angle, const glm::vec3& axis) {
									return glm::rotate(m, angle, axis);
								},
								"scale", sol::overload(
									[](const glm::mat4& m, const glm::vec3& v) { return glm::scale(m, v); },
									[](const glm::mat4& m, float s) { return glm::scale(m, glm::vec3(s)); }
								),
								
								// Matrix multiplication
								sol::meta_function::multiplication, [](const glm::mat4& a, const glm::mat4& b) { 
									return a * b; 
								},
								
								// Static factory methods
								"identity", []() { return glm::mat4(1.0f); },
								"perspective", [](float fov, float aspect, float near, float far) {
									return glm::perspective(fov, aspect, near, far);
								},
								"ortho", [](float left, float right, float bottom, float top, float near, float far) {
									return glm::ortho(left, right, bottom, top, near, far);
								},
								"lookAt", [](const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
									return glm::lookAt(eye, center, up);
								},
								"to_vec3", [](const glm::mat4& m) { 
									return glm::vec3(m[3][0], m[3][1], m[3][2]); 
								},
								
								// String representation
								sol::meta_function::to_string, [](const glm::mat4& m) {
									return "Mat4[" + std::to_string(m[0][0]) + ", " + std::to_string(m[0][1]) + ", " + std::to_string(m[0][2]) + ", " + std::to_string(m[0][3]) + "]";
								});

	return 0;
}