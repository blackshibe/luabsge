---@diagnostic disable: undefined-global

-- Create VFX effect for raytracing
local raytracer_effect = VFXEffect.new()
raytracer_effect:load_fragment_shader("shader/raytracer/frag_default.glsl")

if not raytracer_effect.is_valid then
	error("rt shader is invalid")
end

-- Setup camera (still needed for window system)
local camera = Camera.new()
camera.fov = 70
camera.near_clip = 0.1
camera.far_clip = 100

World.rendering.camera = camera

local MOUSE_SENSITIVITY = 0.002

local camera_inputs = {
	r_x = 0,
	r_y = 0,
}

World.rendering.step:connect(function(delta_time)
	local dim = Window.get_window_dimensions()

	camera.position = Mat4.new(1)
		:translate(Vec3.new(0, 0, 10))
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))

	-- Calculate projection matrix and its inverse
	local aspect_ratio = dim.x / dim.y
	local projection_matrix = Mat4.perspective(math.rad(camera.fov), aspect_ratio, camera.near_clip, camera.far_clip)
	local inv_projection_matrix = projection_matrix:inverse()

	-- Camera to world matrix (inverse of view matrix)
	-- camera.position is already the transform matrix (camera-to-world)
	local camera_to_world_matrix = camera.position

	-- Extract camera position from the camera-to-world matrix
	-- For a simple translation matrix, we can extract the position from the matrix
	-- The translation is typically in the 4th column of the matrix
	-- Since camera.position is our transform matrix, we can use it directly for camera_to_world
	-- For camera position, if it's a simple translation, we can extract from the matrix
	local camera_pos = Vec3.new(0, 0, -10) -- Use the same position we set for the camera

	-- Set uniforms for raytracer
	raytracer_effect:set_uniform_mat4("camera_inv_proj", inv_projection_matrix)
	raytracer_effect:set_uniform_mat4("camera_to_world", camera_to_world_matrix)
	raytracer_effect:set_uniform_vec3("camera_position", camera_pos.x, camera_pos.y, camera_pos.z)
	raytracer_effect:render()

	-- ImGui controls
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()
	end
	ImGui.End()

	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY
	end
end)
