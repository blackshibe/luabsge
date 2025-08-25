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
	r_x = 2,
	r_y = 0.25,
	pos_x = 0,
	pos_y = 0,
	pos_z = -5,
}

for i = 1, 64 do
	TEMP_make_sphere(
		Vec3.new(math.random(-20, 20), 0, math.random(-20, 20)),
		Vec3.new(math.random(0, 100) / 100, math.random(0, 100) / 100, math.random(0, 100) / 100),
		math.random(1, 2),
		false
	)
end

for i = 1, 64 do
	TEMP_make_sphere(
		Vec3.new(math.random(-20, 20), 0, math.random(-20, 20)),
		Vec3.new(1, 1, 1),
		math.random(1, 2) / 2,
		true
	)
end

-- TODO less ai garbage controls
World.rendering.step:connect(function(delta_time)
	local dim = Window.get_window_dimensions()

	camera.matrix = Mat4.new(1)
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))
		:translate(Vec3.new(0, 0, -10))

	-- Calculate projection matrix and its inverse
	local aspect_ratio = dim.x / dim.y
	local projection_matrix = Mat4.perspective(math.rad(camera.fov), aspect_ratio, camera.near_clip, camera.far_clip)
	local inv_projection_matrix = projection_matrix:inverse()

	-- Camera to world matrix (inverse of view matrix)
	-- camera.matrix is already the transform matrix (camera-to-world)
	local camera_to_world_matrix = camera.matrix

	-- Use the actual camera position from our input state
	local camera_final_position = camera.matrix:to_vec3()

	-- Set uniforms for raytracer

	TEMP_get_tbo_texture()
	raytracer_effect:set_uniform_int("spheres_texture", 0)
	raytracer_effect:set_uniform_int("sphere_texture_count", TEMP_get_tbo_texture_count())

	raytracer_effect:set_uniform_mat4("camera_inv_proj", inv_projection_matrix)
	raytracer_effect:set_uniform_mat4("camera_to_world", camera_to_world_matrix)
	raytracer_effect:set_uniform_vec3(
		"camera_position",
		camera_final_position.x,
		camera_final_position.y,
		camera_final_position.z
	)
	raytracer_effect:render()

	-- ImGui controls
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()

		ImGui.Text("TEMP_get_tbo_texture_count(): " .. string.format("%i", TEMP_get_tbo_texture_count()))
		ImGui.Separator()

		ImGui.Text("camera rotation")
		ImGui.Text("x, y = " .. string.format("%.2f, %.2f", math.deg(camera_inputs.r_x), math.deg(camera_inputs.r_y)))
		ImGui.Separator()
	end
	ImGui.End()

	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY
	end
end)
