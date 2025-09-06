#include "camera.h"

void camera_set_shader_projection_matrix(sol::state &lua, BSGEWindow *context_window) {
	sol::table world = lua["World"];
	sol::table rendering = world["rendering"];
	sol::optional<BSGECameraMetadata*> camera_opt = rendering["camera"];

	BSGECameraMetadata* camera = camera_opt.value();
	glm::mat4 camera_projection = camera_get_projection_matrix(*camera);

	glUniformMatrix4fv(glGetUniformLocation(context_window->default_shader, "camera_transform"), 1, GL_FALSE, glm::value_ptr(camera->transform));
	glUniformMatrix4fv(glGetUniformLocation(context_window->default_shader, "projection"), 1, GL_FALSE, glm::value_ptr(camera_projection));
}

static glm::vec2 current_buffer_dimensions;
glm::vec2 get_current_buffer_dimensions() {
	return glm::vec2(current_buffer_dimensions.x, current_buffer_dimensions.y);
}

void set_current_buffer_dimensions(glm::vec2 dimensions) {
	current_buffer_dimensions = dimensions;
}

glm::mat4 camera_get_projection_matrix(BSGECameraMetadata camera) {
	glm::vec2 dimensions = get_current_buffer_dimensions();
	glm::mat4 camera_projection = glm::perspective(glm::radians(camera.fov), (float)dimensions.x / (float)dimensions.y, camera.near_clip, camera.far_clip);

	return camera_projection;
}

glm::mat4 camera_get_projection_matrix_for_resolution(BSGECameraMetadata camera, float x, float y) {
	glm::mat4 camera_projection = glm::perspective(glm::radians(camera.fov), x / y, camera.near_clip, camera.far_clip);

	return camera_projection;
}

void lua_bsge_init_camera(sol::state &lua) {
	lua.new_usertype<BSGECameraMetadata>(
		"Camera",
		sol::constructors<BSGECameraMetadata()>(),
		"fov", &BSGECameraMetadata::fov,
		"near_clip", &BSGECameraMetadata::near_clip,
		"far_clip", &BSGECameraMetadata::far_clip,
		"transform", &BSGECameraMetadata::transform,
		"get_projection_matrix", camera_get_projection_matrix,
		"get_projection_matrix_for_resolution", camera_get_projection_matrix_for_resolution
	);
}
