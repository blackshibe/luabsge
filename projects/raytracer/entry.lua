---@diagnostic disable: undefined-global

-- Create VFX effect for raytracing
local raytracer_effect = VFXEffect.new()
raytracer_effect:load_fragment_shader("shader/raytracer.frag")

-- Setup camera (still needed for window system)
local camera = Camera.new()
camera.fov = 70
camera.near_clip = 0.1
camera.far_clip = 100

World.rendering.camera = camera
camera.position = Mat4.new(1)

-- Mouse tracking
local mouse_pos = Vec2.new(0, 0)

World.rendering.step:connect(function(delta_time)
	local dim = Window.get_window_dimensions()
	raytracer_effect:render()

	-- ImGui controls
	if ImGui.Begin("LuaBSGE Ray Tracer") then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()

		-- Mouse position display
		ImGui.Text("Mouse: " .. string.format("%.0f, %.0f", mouse_pos.x, mouse_pos.y))

		-- Simple mouse interaction
		if ImGui.Button("Center Mouse") then
			mouse_pos = Vec2.new(dim.x * 0.5, dim.y * 0.5)
		end

		if ImGui.Button("Random Mouse") then
			mouse_pos = Vec2.new(math.random() * dim.x, math.random() * dim.y)
		end
	end
	ImGui.End()
end)
