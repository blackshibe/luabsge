#include "camera.h"

int camera_index(lua_State* L) {
	// this, index

	return 0;
}

int camera_newindex(lua_State* L) {
	const char* index = lua_tostring(L, 2);

	if (strcmp(index, "position") == 0) {
		BSGECameraMetadata* camera = (BSGECameraMetadata*)lua_touserdata(L, 1);
		glm::mat4* position = (glm::mat4*)lua_touserdata(L, 3);
		camera->position = *position;

		// printf("set camera position to %f %f %f %f\n", camera->position[0], camera->position[1], camera->position[2], camera->position[3]); 

		return 0;
	}


	return 0;
}

const luaL_Reg bsge_lua_camera_metatable_access[] = {
	{"__index", camera_index},
	{"__newindex", camera_newindex},

	{NULL, NULL},
};

int lua_bsge_instance_camera(lua_State* L) {
	lua_newuserdata(L, sizeof(BSGECameraMetadata) * 2);
	luaL_getmetatable(L, "Camera");
	lua_setmetatable(L, -2);

	// define default values to avoid explosions
	BSGECameraMetadata* camera = (BSGECameraMetadata*)lua_touserdata(L, -1);
	camera->far_clip = 100.0f;
	camera->near_clip = 0.1f;
	camera->fov = 90.0f;
	camera->position = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));

	return 1;
}

void lua_bsge_init_camera(lua_State* L) {
	luaL_newmetatable(L, "Camera");
	luaL_setfuncs(L, bsge_lua_camera_metatable_access, 0);

	// todo: replace with a universal constructor
	lua_register(L, "_temp_Camera", lua_bsge_instance_camera);
}
