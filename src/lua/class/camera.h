#include <lua.hpp>
#include "../../opengl/freetype.h"

// projection is calculated in the render step
struct BSGECameraMetadata {
	glm::mat4 position;
	
	float fov;
	float near_clip;
	float far_clip;
};

void lua_bsge_init_camera(lua_State* L);