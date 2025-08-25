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
		camera->position = *position;

		return 0;
	}

	return 0;
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
										 "position", &BSGECameraMetadata::position
	);
}
