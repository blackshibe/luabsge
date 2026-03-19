---@diagnostic disable: undefined-global

World.rendering.camera = Camera.new()
World.rendering.camera.fov = 90 -- degrees
World.rendering.camera.near_clip = 0.1
World.rendering.camera.far_clip = 100
World.rendering.camera.transform = Mat4.new(1)

function render_pass()
	-- TODO 2d drawing for gizmos
end

World.rendering.step:connect(function(delta_time)
	render_pass()

	local dimensions = Window.get_window_dimensions()
	ImGui.SetNextWindowPos(0, 0)
	ImGui.SetNextWindowSize(dimensions.x, dimensions.y)

	local window_flags = ImGui.WindowFlags_NoTitleBar
		+ ImGui.WindowFlags_NoResize
		+ ImGui.WindowFlags_NoMove
		+ ImGui.WindowFlags_NoCollapse
		+ ImGui.WindowFlags_NoBringToFrontOnFocus

	if ImGui.Begin("LuaBSGE ImGui Demo", window_flags) then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()
	end
	ImGui.End()
end)

print("Lua entry file finished")
