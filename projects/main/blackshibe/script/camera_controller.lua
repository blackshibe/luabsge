-- Common camera controller for various camera behaviors
local Spring = require("script.spring")

local CameraController = {}
CameraController.__index = CameraController

-- Create a new camera controller
function CameraController.new(camera)
	local self = setmetatable({}, CameraController)
	self.camera = camera or Camera.new()

	-- Mouse tracking
	self.mouse_sensitivity = 0.002
	self.last_mouse_position = Vec2.new(0, 0)

	-- Springs for smooth movement
	self.x_spring = Spring.new(0.5, 100, 50)
	self.y_spring = Spring.new(0.5, 100, 50)
	self.z_spring = Spring.new(0, 100, 50)

	-- Rotation tracking
	self.rotation = {
		x = 1.9,
		y = 0.1,
	}

	-- Position
	self.position = Vec3.new(0, 0, -5)
	self.base_matrix = Mat4.new(1)

	return self
end

-- Mouse-based camera control (like entry.lua)
function CameraController:update_mouse_tracking(delta_time)
	local mouse_position = World.input.get_mouse_position()
	local mouse_delta = mouse_position - self.last_mouse_position
	self.last_mouse_position = mouse_position

	local window_dims = Window.get_window_dimensions()
	local mouse_position_scaled = self.last_mouse_position / window_dims

	self.x_spring.target = mouse_position_scaled.x
	self.y_spring.target = mouse_position_scaled.y

	local camera_x = self.x_spring:update(delta_time)
	local camera_y = self.y_spring:update(delta_time)

	-- Apply smooth camera movement
	self.camera.transform = Mat4.new(1):translate(Vec3.new(-camera_x + 0.5, camera_y - 0.5, -4) * 0.25)

	return mouse_delta
end

-- FPS-style camera control (like other-scene.lua)
function CameraController:update_fps_controls(delta_time)
	if World.input.is_right_mouse_down() then
		local delta = World.input.get_mouse_delta()
		self.rotation.x = self.rotation.x + delta.x * self.mouse_sensitivity
		self.rotation.y = self.rotation.y + delta.y * self.mouse_sensitivity
	end

	local camera_z = self.z_spring:update(delta_time)

	self.camera.transform = Mat4.new(1)
		:translate(Vec3.new(self.position.x, self.position.y, self.position.z + camera_z))
		:rotate(self.rotation.y, Vec3.new(1, 0, 0))
		:rotate(self.rotation.x, Vec3.new(0, 1, 0))
end

-- Set camera position
function CameraController:set_position(pos)
	self.position = pos
end

-- Set camera rotation
function CameraController:set_rotation(x, y)
	self.rotation.x = x or self.rotation.x
	self.rotation.y = y or self.rotation.y
end

-- Set mouse sensitivity
function CameraController:set_mouse_sensitivity(sensitivity)
	self.mouse_sensitivity = sensitivity
end

-- Set spring parameters for smooth movement
function CameraController:set_spring_params(stiffness, damping)
	self.x_spring.stiffness = stiffness or self.x_spring.stiffness
	self.x_spring.damping = damping or self.x_spring.damping
	self.y_spring.stiffness = stiffness or self.y_spring.stiffness
	self.y_spring.damping = damping or self.y_spring.damping
	self.z_spring.stiffness = stiffness or self.z_spring.stiffness
	self.z_spring.damping = damping or self.z_spring.damping
end

-- Initialize camera with standard settings
function CameraController:setup_camera()
	self.camera.fov = 70
	self.camera.near_clip = 0.1
	self.camera.far_clip = 100
	self.camera.transform = self.base_matrix
	World.rendering.camera = self.camera
end

return CameraController
