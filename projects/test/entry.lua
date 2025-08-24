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

local next_check = now() + 1
local fps = 0
local i = 0
local time_ms = 0

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
	local pos = Vec3.new(math.sin(now() * 0.001) * 0.5, 0, -1)
	camera.position = base_matrix:translate(Vec3.new(0, 0, -1))

	to_render[2]:render()

	-- to_render[2]:render()

	-- if now() > next_check then
	-- 	textlabel.text = string.format("memory: %.2f kb", collectgarbage("count"))

	-- 	next_check = now() + 2000
	-- 	fps = 0

	-- 	i = i + 1
	-- end
end)

-- print("Lua entry file finished")
