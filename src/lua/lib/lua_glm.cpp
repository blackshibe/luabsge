#include "lua_glm.h"

#include <cstdio>
#include <string>

namespace Lua::transform {

	void print_mat4(glm::mat4 m) {
		printf("%f, %f, %f\n %f, %f, %f\n %f, %f, %f\n",
		       m[0][0], m[0][1], m[0][2],
		       m[1][0], m[1][1], m[1][2],
		       m[2][0], m[2][1], m[2][2],
		       m[3][0], m[3][1], m[3][2]);
	}

	glm::vec3 mat4_to_vec3(glm::mat4 m) {
		return glm::vec3(m[3][0], m[3][1], m[3][2]);
	}

	glm::vec4 mat4_to_vec4(glm::mat4 m) {
		return glm::vec4(m[3][0], m[3][1], m[3][2], m[3][3]);
	}

	glm::vec3 mat4_to_vec3_scale(glm::mat4 m) {
		glm::vec3 xAxis = glm::vec3(m[0]);
		glm::vec3 yAxis = glm::vec3(m[1]);
		glm::vec3 zAxis = glm::vec3(m[2]);

		glm::vec3 scale;
		scale.x = glm::length(xAxis);
		scale.y = glm::length(yAxis);
		scale.z = glm::length(zAxis);

		return scale;
	}

	void init(sol::state &lua) {
		lua.new_usertype<glm::vec2>("Vec2", sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y,

		                            sol::meta_function::addition, [](const glm::vec2 &a, const glm::vec2 &b) { return a + b; }, sol::meta_function::subtraction, [](const glm::vec2 &a, const glm::vec2 &b) { return a - b; }, sol::meta_function::multiplication, sol::overload([](const glm::vec2 &a, const glm::vec2 &b) { return a * b; }, [](const glm::vec2 &a, float b) { return a * b; }, [](float a, const glm::vec2 &b) { return a * b; }), sol::meta_function::division, sol::overload([](const glm::vec2 &a, const glm::vec2 &b) { return a / b; }, [](const glm::vec2 &a, float b) { return a / b; }), sol::meta_function::unary_minus, [](const glm::vec2 &v) { return -v; },

		                            "length", [](const glm::vec2 &v) { return glm::length(v); }, "normalize", [](const glm::vec2 &v) { return glm::normalize(v); }, "dot", [](const glm::vec2 &a, const glm::vec2 &b) { return glm::dot(a, b); }, "distance", [](const glm::vec2 &a, const glm::vec2 &b) { return glm::distance(a, b); },

		                            sol::meta_function::to_string, [](const glm::vec2 &v) { return "Vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")"; });

		lua.new_usertype<glm::vec3>("Vec3", sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(), "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z,

		                            sol::meta_function::addition, [](const glm::vec3 &a, const glm::vec3 &b) { return a + b; }, sol::meta_function::subtraction, [](const glm::vec3 &a, const glm::vec3 &b) { return a - b; }, sol::meta_function::multiplication, sol::overload([](const glm::vec3 &a, const glm::vec3 &b) { return a * b; }, [](const glm::vec3 &a, float b) { return a * b; }, [](float a, const glm::vec3 &b) { return a * b; }), sol::meta_function::division, sol::overload([](const glm::vec3 &a, const glm::vec3 &b) { return a / b; }, [](const glm::vec3 &a, float b) { return a / b; }), sol::meta_function::unary_minus, [](const glm::vec3 &v) { return -v; },

		                            "length", [](const glm::vec3 &v) { return glm::length(v); }, "normalize", [](const glm::vec3 &v) { return glm::normalize(v); }, "dot", [](const glm::vec3 &a, const glm::vec3 &b) { return glm::dot(a, b); }, "cross", [](const glm::vec3 &a, const glm::vec3 &b) { return glm::cross(a, b); }, "distance", [](const glm::vec3 &a, const glm::vec3 &b) { return glm::distance(a, b); },

		                            sol::meta_function::to_string, [](const glm::vec3 &v) { return "Vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")"; });

		lua.new_usertype<glm::vec4>("Vec4",
		                            sol::constructors<glm::vec4(), glm::vec4(float), glm::vec4(float, float, float, float)>(),
		                            "x", &glm::vec4::x,
		                            "y", &glm::vec4::y,
		                            "z", &glm::vec4::z,
		                            "w", &glm::vec4::w,

		                            sol::meta_function::to_string, [](const glm::vec4 &v) {
			                            return "Vec4(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
		                            });

		lua.new_usertype<glm::mat4>("Mat4", sol::constructors<glm::mat4(), glm::mat4(float)>(),

		                            "translate", [](const glm::mat4 &m, const glm::vec3 &v) { return glm::translate(m, v); },

		                            "inverse", [](const glm::mat4 &m) { return glm::inverse(m); },

		                            "rotate", [](const glm::mat4 &m, float angle, const glm::vec3 &axis) { return glm::rotate(m, angle, axis); }, "scale", sol::overload([](const glm::mat4 &m, const glm::vec3 &v) { return glm::scale(m, v); }, [](const glm::mat4 &m, float s) { return glm::scale(m, glm::vec3(s)); }),

		                            "lerp", sol::overload([](const glm::mat4 &m, const glm::mat4 &v, float a) {
											glm::quat firstQuat = glm::quat_cast(m);
											glm::quat otherQuat = glm::quat_cast(v);

											return glm::mat4_cast(glm::lerp(firstQuat, otherQuat, a)); }),

		                            sol::meta_function::multiplication, [](const glm::mat4 &a, const glm::mat4 &b) { return a * b; },

		                            "identity", []() { return glm::mat4(1.0f); }, "perspective", [](float fov, float aspect, float near, float far) { return glm::perspective(fov, aspect, near, far); }, "ortho", [](float left, float right, float bottom, float top, float near, float far) { return glm::ortho(left, right, bottom, top, near, far); }, "lookAt", [](const glm::vec3 &eye, const glm::vec3 &center, const glm::vec3 &up) { return glm::lookAt(eye, center, up); }, "to_vec3", [](const glm::mat4 &m) { return mat4_to_vec3(m); },

		                            sol::meta_function::to_string, [](const glm::mat4 &m) { return "Mat4[" + std::to_string(m[0][0]) + ", " + std::to_string(m[0][1]) + ", " + std::to_string(m[0][2]) + ", " + std::to_string(m[0][3]) + "]"; });

		auto transform_namespace = lua["Transform"].get_or_create<sol::table>();
		transform_namespace.set_function("new", [](const glm::vec3 &position) { return glm::translate(glm::mat4(1.0f), position); });
	}
}
