#include "camera.h"

int camera_index(lua_State *L) {
	// this, index

	return 0;
}

int camera_newindex(lua_State *L) {
	const char *index = lua_tostring(L, 2);

	if (strcmp(index, "position") == 0) {
		BSGECameraMetadata *camera = (BSGECameraMetadata *)lua_touserdata(L, 1);
		glm::mat4 *position = (glm::mat4 *)lua_touserdata(L, 3);
		camera->matrix = *position;

		return 0;
	}

	return 0;
}

glm::mat4 camera_get_projection_matrix(BSGECameraMetadata camera) {
	glm::vec2 dimensions = get_window_dimensions();
	glm::mat4 camera_projection = glm::perspective(glm::radians(camera.fov), (float)dimensions.x / (float)dimensions.y, camera.near_clip, camera.far_clip);

	return camera_projection;
}

const luaL_Reg bsge_lua_camera_metatable_access[] = {
	{"__index", camera_index},
	{"__newindex", camera_newindex},

	{NULL, NULL},
};

void lua_bsge_init_camera(sol::state &lua) {
	lua.new_usertype<BSGECameraMetadata>("Camera",
										 sol::constructors<BSGECameraMetadata()>(),
										 "fov", &BSGECameraMetadata::fov,
										 "near_clip", &BSGECameraMetadata::near_clip,
										 "far_clip", &BSGECameraMetadata::far_clip,
										 "matrix", &BSGECameraMetadata::matrix,
										 "get_projection_matrix", camera_get_projection_matrix
	);
}
