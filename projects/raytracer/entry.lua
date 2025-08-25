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

for i = 1, 32 do
	TEMP_make_sphere(
		Vec3.new(math.random(-20, 20), -10, math.random(-20, 20)),
		Vec3.new(math.random(0, 100) / 100, math.random(0, 100) / 100, math.random(0, 100) / 100),
		math.random(1, 2),
		0
	)
end

for i = 1, 16 do
	TEMP_make_sphere(
		Vec3.new(math.random(-20, 20), -10, math.random(-20, 20)),
		Vec3.new(1, 1, 1),
		math.random(1, 2) / 2,
		math.random(1, 100) / 100
	)
end

for i = 1, 16 do
	TEMP_make_sphere(
		Vec3.new(math.random(-20, 20), -10, math.random(-20, 20)),
		Vec3.new(math.random(0, 1), math.random(0, 1), math.random(0, 1)),
		math.random(1, 2) / 2,
		math.random(1, 100) / 50
	)
end

TEMP_make_sphere(Vec3.new(0, 0, 0), Vec3.new(1, 0, 0), 1, 0)
TEMP_make_sphere(Vec3.new(0, 2, 0), Vec3.new(1, 1, 1), 0.5, 2.0)

local sample_count = 8
local recompile_time = now()
World.rendering.step:connect(function(delta_time)
	-- raytracer_effect:render()
	Gizmo.set_line_width(1)
	Gizmo.set_depth_test(false)

	Gizmo.draw_grid(10, 10, Vec3.new(0.25, 0.25, 0.25))
	Gizmo.set_line_width(5)

	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))

	camera.matrix = Mat4.new(1)
		:translate(Vec3.new(0, 0, -10))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))

	-- Calculate projection matrix and its inverse
	local inv_projection_matrix = camera:get_projection_matrix():inverse()

	-- Camera to world matrix (inverse of view matrix)
	-- camera.matrix is already the transform matrix (camera-to-world)
	local camera_to_world_matrix = camera.matrix:inverse()

	-- Use the actual camera position from our input state
	local camera_final_position = camera_to_world_matrix:to_vec3()

	TEMP_get_tbo_texture()
	raytracer_effect:set_uniform_int("spheres_texture", 0)
	raytracer_effect:set_uniform_int("sphere_texture_count", TEMP_get_tbo_texture_count())
	raytracer_effect:set_uniform_int("sample_count", sample_count)

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

		local changed, value = ImGui.SliderInt("Light Bounces", sample_count, 4, 512)
		if changed then
			sample_count = value
		end

		if ImGui.Button("Recompile") or World.input.is_key_down(KEY_F) and math.abs(recompile_time - now()) > 100 then
			recompile_time = now()
			raytracer_effect:load_fragment_shader("shader/raytracer/frag_default.glsl")
		end

		ImGui.PushTextColor(Vec4.new(0.5, 0.5, 1, 1))
		ImGui.Text("recompiled at " .. recompile_time)
		ImGui.PopStyleColor()
	end
	ImGui.End()

	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY
	end
end)
