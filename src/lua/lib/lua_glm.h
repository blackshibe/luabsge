#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <lua.hpp>
#include <sol/sol.hpp>
#include <string>

namespace Lua::transform {
	void init(sol::state &lua);

	void print_mat4(glm::mat4 m);
	glm::vec4 mat4_to_vec4(glm::mat4 m);
	glm::vec3 mat4_to_vec3(glm::mat4 m);
	glm::vec3 mat4_to_vec3_scale(glm::mat4 m);
}
