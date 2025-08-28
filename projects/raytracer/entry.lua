local scene = require("scene-box")

-- Create VFX effect for raytracing
local raytracer_effect = VFXEffect.new()
raytracer_effect:load_fragment_shader("shader/raytracer/frag_default.glsl")

local framebuffer_resolution = Vec2.new(1000, 1000)
local raytracer_framebuffer = Framebuffer.new(framebuffer_resolution.x, framebuffer_resolution.y)
local accum_frames = 0

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
	r_x = 1.9,
	r_y = 0.1,
	pos_x = 0,
	pos_y = 0,
	pos_z = -5,
}

local sample_count = 2
local bounce_count = 2
local use_debug = false
local hot_reloading = true
local recompile_time = now()

function draw_bounding_box(min, max)
	local color = Vec3.new(1, 0, 0) -- Red color

	-- Bottom face (z = min.z)
	Gizmo.draw_line(Vec3.new(min.x, min.y, min.z), Vec3.new(max.x, min.y, min.z), color)
	Gizmo.draw_line(Vec3.new(max.x, min.y, min.z), Vec3.new(max.x, max.y, min.z), color)
	Gizmo.draw_line(Vec3.new(max.x, max.y, min.z), Vec3.new(min.x, max.y, min.z), color)
	Gizmo.draw_line(Vec3.new(min.x, max.y, min.z), Vec3.new(min.x, min.y, min.z), color)

	-- Top face (z = max.z)
	Gizmo.draw_line(Vec3.new(min.x, min.y, max.z), Vec3.new(max.x, min.y, max.z), color)
	Gizmo.draw_line(Vec3.new(max.x, min.y, max.z), Vec3.new(max.x, max.y, max.z), color)
	Gizmo.draw_line(Vec3.new(max.x, max.y, max.z), Vec3.new(min.x, max.y, max.z), color)
	Gizmo.draw_line(Vec3.new(min.x, max.y, max.z), Vec3.new(min.x, min.y, max.z), color)

	-- Vertical edges connecting bottom and top faces
	Gizmo.draw_line(Vec3.new(min.x, min.y, min.z), Vec3.new(min.x, min.y, max.z), color)
	Gizmo.draw_line(Vec3.new(max.x, min.y, min.z), Vec3.new(max.x, min.y, max.z), color)
	Gizmo.draw_line(Vec3.new(max.x, max.y, min.z), Vec3.new(max.x, max.y, max.z), color)
	Gizmo.draw_line(Vec3.new(min.x, max.y, min.z), Vec3.new(min.x, max.y, max.z), color)

	Gizmo.draw_line(Vec3.new(min.x, min.y, min.z), Vec3.new(min.x, min.y - 1, min.z), Vec3.new(1, 1, 1))
	Gizmo.draw_line(Vec3.new(max.x, max.y, max.z), Vec3.new(max.x, max.y + 1, max.z), Vec3.new(1, 1, 1))
end

local next_update = now() + 1000
World.rendering.step:connect(function(delta_time)
	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY

		raytracer_framebuffer:clear()
		accum_frames = 0
	end

	Gizmo.set_line_width(0.01)
	Gizmo.set_depth_test(false)

	Gizmo.draw_grid(10, 10, Vec3.new(0.25, 0.25, 0.25))

	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))

	for i, v in pairs(scene.meshes) do
		local bb = v:get_bounding_box(v.mesh)
		draw_bounding_box(bb.world_min, bb.world_max)
	end

	camera.matrix = Mat4.new(1)
		:translate(Vec3.new(0, 0, -8))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))

	scene.monkey.matrix = Mat4.new(1)
		:rotate(math.pi + 1, Vec3.new(0, 1, 0))
		:translate(Vec3.new(0, 0, -0.5))
		:scale(1)
		:rotate((now() / 3000) % math.pi * 2, Vec3.new(1, 0, 0))
	scene.monkey:update()

	local bb = scene.monkey:get_bounding_box(scene.monkey.mesh)
	draw_bounding_box(bb.world_min, bb.world_max)

	-- Calculate projection matrix and its inverse
	local inv_projection_matrix =
		camera:get_projection_matrix_for_resolution(framebuffer_resolution.x, framebuffer_resolution.y):inverse()

	-- Camera to world matrix (inverse of view matrix)
	-- camera.matrix is already the transform matrix (camera-to-world)
	local camera_to_world_matrix = camera.matrix:inverse()

	-- Use the actual camera position from our input state
	local camera_final_position = camera_to_world_matrix:to_vec3()

	SphereBufferObject.bind_textures(GL_TEXTURE0)
	MeshBufferObject.bind_textures(GL_TEXTURE1, GL_TEXTURE2)
	raytracer_framebuffer:bind_texture(GL_TEXTURE3)

	raytracer_effect:set_uniform_int("spheres_texture", 0) -- texture 0
	raytracer_effect:set_uniform_int("mesh_triangle_data", 1) -- texture 1
	raytracer_effect:set_uniform_int("mesh_data", 2) -- texture 2
	raytracer_effect:set_uniform_int("previous_frame", 3)
	raytracer_effect:set_uniform_int("sphere_texture_count", SphereBufferObject.get_count())
	raytracer_effect:set_uniform_int("mesh_count", MeshBufferObject.get_count())
	raytracer_effect:set_uniform_int("accum_frames", accum_frames)

	raytracer_effect:set_uniform_int("sample_count", sample_count)
	raytracer_effect:set_uniform_int("bounce_count", bounce_count)
	raytracer_effect:set_uniform_int("use_debug", use_debug and 1 or 0)

	raytracer_effect:set_uniform_mat4("camera_inv_proj", inv_projection_matrix)
	raytracer_effect:set_uniform_mat4("camera_to_world", camera_to_world_matrix)
	raytracer_effect:set_uniform_vec3(
		"camera_position",
		camera_final_position.x,
		camera_final_position.y,
		camera_final_position.z
	)

	if raytracer_effect.is_valid then
		raytracer_framebuffer:bind()
		raytracer_effect:render()
		raytracer_framebuffer:unbind()
		accum_frames = accum_frames + 1
	end

	-- ImGui controls
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.Image(raytracer_framebuffer.texture_id, Vec2.new(800, 600))
		ImGui.Separator()
		ImGui.Spacing()

		ImGui.PushTextColor(Vec4.new(0.8, 0.9, 1.0, 1.0))
		ImGui.Text("Stats")
		ImGui.PopStyleColor()

		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()
		ImGui.Spacing()

		ImGui.Text("Frames accumulated: " .. string.format("%.1f", accum_frames))
		ImGui.Separator()
		ImGui.Spacing()

		ImGui.PushTextColor(Vec4.new(0.8, 0.9, 1.0, 1.0))
		ImGui.Text("Settings")
		ImGui.PopStyleColor()

		local s_changed, value1 = ImGui.SliderInt("Light Samples", sample_count, 1, 128)
		local b_changed, value2 = ImGui.SliderInt("Light Bounces", bounce_count, 1, 16)
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

		local changed, val = ImGui.Checkbox("Hot reload constantly", hot_reloading)
		local debug_changed, val2 = ImGui.Checkbox("Debug", use_debug)
		if changed then
			hot_reloading = val
		end

		if ((World.input.is_key_down(KEY_F)) or hot_reloading) and math.abs(recompile_time - now()) > 200 then
			recompile_time = now()
			raytracer_effect:load_fragment_shader("shader/raytracer/frag_default.glsl")
		end

		if debug_changed then
			use_debug = val2
		end

		ImGui.PushTextColor(raytracer_effect.is_valid and Vec4.new(0.5, 0.5, 1, 1) or Vec4.new(1.0, 0, 0, 1))
		ImGui.Text("Recompiled @ " .. recompile_time)
		ImGui.Text("Valid = " .. tostring(raytracer_effect.is_valid))
		ImGui.Text("Press F to recompile")

		ImGui.PopStyleColor()
	end
	ImGui.End()
end)
