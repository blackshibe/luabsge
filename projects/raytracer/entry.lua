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
local MOVE_SPEED = 5.0

local camera_inputs = {
	r_x = 0,
	r_y = 0,
	pos_x = 0,
	pos_y = 0,
	pos_z = 10,
}

-- TODO less ai garbage controls
World.rendering.step:connect(function(delta_time)
	local dim = Window.get_window_dimensions()

	-- Handle WASD movement
	local move_delta = MOVE_SPEED * delta_time

	if World.input.is_key_down(KEY_W) then
		camera_inputs.pos_z = camera_inputs.pos_z - move_delta
	end
	if World.input.is_key_down(KEY_S) then
		camera_inputs.pos_z = camera_inputs.pos_z + move_delta
	end
	if World.input.is_key_down(KEY_A) then
		camera_inputs.pos_x = camera_inputs.pos_x - move_delta
	end
	if World.input.is_key_down(KEY_D) then
		camera_inputs.pos_x = camera_inputs.pos_x + move_delta
	end
	if World.input.is_key_down(KEY_Q) then
		camera_inputs.pos_y = camera_inputs.pos_y + move_delta
	end
	if World.input.is_key_down(KEY_E) then
		camera_inputs.pos_y = camera_inputs.pos_y - move_delta
	end

	camera.position = Mat4.new(1)
		:translate(Vec3.new(camera_inputs.pos_x, camera_inputs.pos_y, camera_inputs.pos_z))
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))

	-- Calculate projection matrix and its inverse
	local aspect_ratio = dim.x / dim.y
	local projection_matrix = Mat4.perspective(math.rad(camera.fov), aspect_ratio, camera.near_clip, camera.far_clip)
	local inv_projection_matrix = projection_matrix:inverse()

	-- Camera to world matrix (inverse of view matrix)
	-- camera.position is already the transform matrix (camera-to-world)
	local camera_to_world_matrix = camera.position

	-- Use the actual camera position from our input state
	local camera_pos = Vec3.new(camera_inputs.pos_x, camera_inputs.pos_y, camera_inputs.pos_z)

	-- Set uniforms for raytracer
	raytracer_effect:set_uniform_mat4("camera_inv_proj", inv_projection_matrix)
	raytracer_effect:set_uniform_mat4("camera_to_world", camera_to_world_matrix)
	raytracer_effect:set_uniform_vec3("camera_position", camera_pos.x, camera_pos.y, camera_pos.z)
	raytracer_effect:render()

	-- ImGui controls
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()

		-- Camera info
		ImGui.Text("Camera Position:")
		ImGui.Text("  X: " .. string.format("%.2f", camera_inputs.pos_x))
		ImGui.Text("  Y: " .. string.format("%.2f", camera_inputs.pos_y))
		ImGui.Text("  Z: " .. string.format("%.2f", camera_inputs.pos_z))

		ImGui.Text("Camera Rotation:")
		ImGui.Text("  Yaw: " .. string.format("%.2f", math.deg(camera_inputs.r_x)))
		ImGui.Text("  Pitch: " .. string.format("%.2f", math.deg(camera_inputs.r_y)))

		ImGui.Separator()

		-- Controls help
		ImGui.Text("Controls:")
		ImGui.Text("WASD - Move")
		ImGui.Text("Q/E - Up/Down")
		ImGui.Text("RMB + Mouse - Look around")
		ImGui.Text("Shift - Speed boost")
	end
	ImGui.End()

	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY
	end
end)
