---@diagnostic disable: undefined-global

local font = Font.new()
font:load("font/FiraCode-Regular.ttf")

local display_label = Textlabel.new()
display_label.font = font

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local texture = Image.new("image/fox.jpg")
local mesh = Mesh.new()
mesh:load("mesh/box.obj")
mesh.texture = texture

local camera_z_spring = require("script.spring").new(0)
local base_matrix = Mat4.new(1)

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
camera.matrix = base_matrix

World.rendering.step:connect(function(delta_time)
	local dim = Window.get_window_dimensions()
	local alpha = math.sin(now() * 0.001) * 0.5
	display_label.position = Vec2.new(dim.x / 2, dim.y / 2)
	display_label.anchor = Vec2.new(0.5, alpha)
	display_label.scale = 0.5 + alpha
	display_label.color = Vec3.new(1, alpha, math.abs(math.sin(now() / 5000)))
	display_label:render()

	local camera_z = camera_z_spring:update(delta_time)
	camera.matrix = Mat4.new(1)
		:translate(Vec3.new(0, 0, -5))
		:rotate(0.25, Vec3.new(1, 0, 0))
		:rotate((now() / 5000) % math.pi * 2, Vec3.new(0, 1, 0))
	mesh.matrix = Mat4.new(1):rotate(camera_z, Vec3.new(1, 0, 0))
	mesh:render()

	Gizmo.set_line_width(0.05)
	Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))

	Gizmo.set_line_width(2)
	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))

	if ImGui.Begin("LuaBSGE ImGui Demo") then
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()

		-- Demo slider
		local changed, slider_val = ImGui.SliderFloat("Test Slider", 0.5, -10, 10)
		if changed then
			camera_z_spring.target = slider_val
		end
	end
	ImGui.End()
end)

print("Lua entry file finished")
