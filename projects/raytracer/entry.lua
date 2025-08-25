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

for i = 1, 24 do
	local obj = SphereBufferObject.new()

	obj.center = Vec3.new(math.random(-20, 20), -10, math.random(-20, 20))
	obj.color = Vec3.new(math.random(0, 100) / 100, math.random(0, 100) / 100, math.random(0, 100) / 100)
	obj.radius = math.random(1, 2)
	obj.emissive = 0
	obj:register()
end

for i = 1, 6 do
	local obj = SphereBufferObject.new()

	obj.center = Vec3.new(math.random(-20, 20), -10, math.random(-20, 20))
	obj.color = Vec3.new(math.random(0, 100) / 100, math.random(0, 100) / 100, math.random(0, 100) / 100)
	obj.radius = math.random(1, 2)
	obj.emissive = 100
	obj:register()
end

local texture = Image.new()
texture:load("image/fox.jpg")

local mesh = Mesh.new()
mesh:load("mesh/box.obj")
mesh.texture = texture

local obj = SphereBufferObject.new():register()
obj.center = Vec3.new(0, 0, 0)
obj.color = Vec3.new(math.random(0, 100) / 100, math.random(0, 100) / 100, math.random(0, 100) / 100)
obj.radius = 0.5
obj.emissive = 100

local obj2 = SphereBufferObject.new()
obj2.center = Vec3.new(5, 0, 0)
obj2.color = Vec3.new(1, 0, 0)
obj2.radius = 2
obj2.emissive = 0
obj2:register()

mesh.position = Mat4.new(1):rotate((now() / 5000) % math.pi, Vec3.new(1, 0, 0))

mesh_tbo_register_mesh(mesh)

local sample_count = 16
local bounce_count = 2
local recompile_time = now()
local always_recompile = true
World.rendering.step:connect(function(delta_time)
	-- raytracer_effect:render()
	Gizmo.set_line_width(1)
	Gizmo.set_depth_test(false)

	Gizmo.draw_grid(10, 10, Vec3.new(0.25, 0.25, 0.25))
	Gizmo.set_line_width(5)

	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))

	obj.center = Vec3.new(2, 0, math.sin(now() / 500))
	obj:update()

	camera.matrix = Mat4.new(1)
		:translate(Vec3.new(0, 0, -6))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))

	-- Calculate projection matrix and its inverse
	local inv_projection_matrix = camera:get_projection_matrix():inverse()

	-- Camera to world matrix (inverse of view matrix)
	-- camera.matrix is already the transform matrix (camera-to-world)
	local camera_to_world_matrix = camera.matrix:inverse()

	-- Use the actual camera position from our input state
	local camera_final_position = camera_to_world_matrix:to_vec3()

	TEMP_bind_tbo_texture()
	mesh_tbo_bind_meshTriangles()

	raytracer_effect:set_uniform_int("spheres_texture", 0) -- texture 0
	raytracer_effect:set_uniform_int("mesh_triangle_data", 1) -- texture 1
	raytracer_effect:set_uniform_int("mesh_data", 2) -- texture 2
	raytracer_effect:set_uniform_int("sphere_texture_count", TEMP_get_tbo_texture_count())
	raytracer_effect:set_uniform_int("mesh_count", mesh_tbo_get_count())

	raytracer_effect:set_uniform_int("sample_count", sample_count)
	raytracer_effect:set_uniform_int("bounce_count", bounce_count)

	raytracer_effect:set_uniform_mat4("camera_inv_proj", inv_projection_matrix)
	raytracer_effect:set_uniform_mat4("camera_to_world", camera_to_world_matrix)
	raytracer_effect:set_uniform_vec3(
		"camera_position",
		camera_final_position.x,
		camera_final_position.y,
		camera_final_position.z
	)

	-- mesh:render()

	if raytracer_effect.is_valid then
		raytracer_effect:render()
	end

	-- ImGui controls
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.PushTextColor(Vec4.new(0.8, 0.9, 1.0, 1.0))
		ImGui.Text("Performance")
		ImGui.PopStyleColor()

		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()
		ImGui.Spacing()

		ImGui.PushTextColor(Vec4.new(0.8, 0.9, 1.0, 1.0))
		ImGui.Text("RT Debug")
		ImGui.PopStyleColor()

		ImGui.Text("TEMP_get_tbo_texture_count(): " .. string.format("%i", TEMP_get_tbo_texture_count()))
		ImGui.Text("mesh_tbo_get_count(): " .. string.format("%i", mesh_tbo_get_count()))
		ImGui.Spacing()

		local s_changed, value1 = ImGui.SliderInt("Light Samples", sample_count, 1, 512)
		local b_changed, value2 = ImGui.SliderInt("Light Bounces", bounce_count, 1, 12)
		if s_changed then
			sample_count = value1
		end

		if b_changed then
			bounce_count = value2
		end

		ImGui.Separator()
		ImGui.Spacing()

		ImGui.PushTextColor(Vec4.new(0.8, 0.9, 1.0, 1.0))
		ImGui.Text("Camera Rotation")
		ImGui.PopStyleColor()

		ImGui.Text("x, y = " .. string.format("%.2f, %.2f", math.deg(camera_inputs.r_x), math.deg(camera_inputs.r_y)))
		ImGui.Spacing()
		ImGui.Separator()

		local changed, val = ImGui.Checkbox("Hot reload constantly", always_recompile)
		if changed then
			always_recompile = val
		end

		if ((World.input.is_key_down(KEY_F)) or always_recompile) and math.abs(recompile_time - now()) > 200 then
			recompile_time = now()
			raytracer_effect:load_fragment_shader("shader/raytracer/frag_default.glsl")
		end

		ImGui.PushTextColor(raytracer_effect.is_valid and Vec4.new(0.5, 0.5, 1, 1) or Vec4.new(1.0, 0, 0, 1))
		ImGui.Text("Recompiled @ " .. recompile_time)
		ImGui.Text("Valid = " .. tostring(raytracer_effect.is_valid))
		ImGui.Text("Press F to recompile")

		ImGui.PopStyleColor()
	end
	ImGui.End()

	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY
	end
end)
