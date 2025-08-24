---@diagnostic disable: undefined-global

local function map(a, b, c, d, e)
	return (a - b) / (c - b) * (e - d) + d
end

local font = Font.new()
font:load("font/FiraCode-Regular.ttf")

local display_label = Textlabel.new()
display_label.font = font

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local objects = {}
local paths = {
	{ "image/fox.jpg", "mesh/box.obj" },
	{ "image/eotek.png", "mesh/sphere.obj" },
}

for i, v in pairs(paths) do
	warn("Loading object", v[1], v[2])
	local texture = Image.new()
	texture:load(v[1])

	local mesh = Mesh.new()
	mesh:load(v[2])
	mesh.texture = texture

	objects[i] = { texture, mesh }
end

local i = 0
local time_ms = 0
local camera_z = 0

local base_matrix = Mat4.new(1)
camera.position = base_matrix

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)
	time_ms = time_ms + (delta_time * 1000)

	local dim = Window.get_window_dimensions()
	local alpha = math.sin(now() * 0.001) * 0.5
	display_label.position = Vec2.new(dim.x / 2, dim.y / 2)
	display_label.anchor = Vec2.new(0.5, alpha)
	display_label.scale = 0.5 + alpha
	display_label.color = Vec3.new(1, alpha, math.abs(math.sin(now() / 5000)))
	-- display_label:render()

	local to_render = objects[(i % #objects) + 1]

	camera.position = base_matrix:translate(Vec3.new(0, 0, camera_z))

	to_render[2]:render()

	-- ImGui demo window
	if ImGui.Begin("LuaBSGE ImGui Demo") then
		ImGui.Text("Hello from Lua ImGui bindings!")
		ImGui.Separator()

		if ImGui.Button("Click me!") then
			warn("Button clicked from ImGui!")
		end

		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Text("Delta time: " .. string.format("%.3f ms", delta_time * 1000))

		local alpha_val = math.sin(now() * 0.001) * 0.5 + 0.5
		ImGui.Text("Animated alpha: " .. string.format("%.2f", alpha_val))

		ImGui.TextColored(1.0, alpha_val, 0.5, 1.0, "Colored text!")

		-- Demo slider
		local changed, slider_val = ImGui.SliderFloat("Test Slider", 0.5, -10, 10)
		if changed then
			camera_z = slider_val
			ImGui.Text("Slider changed to: " .. string.format("%.2f", slider_val))
		end
	end
	ImGui.End()
end)

-- print("Lua entry file finished")
