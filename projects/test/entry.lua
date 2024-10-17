---@diagnostic disable: undefined-global

local function map(a, b, c, d, e)
	return (a - b) / (c - b) * (e - d) + d
end

local font = Font.new()
font:dbg_get_data()
font:load("font/FiraCode-Regular.ttf")

font:dbg_get_data()

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

Template.new()
Template.function_calls = 1

local tbl = {
	1, 1, 1, 1, 1, 5, 5, 5, 2, 1, 5
}

local str_tbl = "Dane: "
for i, v in pairs(tbl) do
	str_tbl = str_tbl .. tostring(v)
	if i ~= #tbl then
		str_tbl = str_tbl .. " "
	end
end

local min_appearances = #tbl / 2
local appearances = {}

for i, v in pairs(tbl) do
	appearances[v] = appearances[v] or 0
	appearances[v] = appearances[v] + 1
end

local winner = "Brak lidera"
for i, v in pairs(appearances) do
	if v > min_appearances then
		winner = "Lider: " .. tostring(i)
		break
	end
end

local steps = {
	"Kalkulowanie lidera!!",
	str_tbl,
	tostring(winner),
}

local state = {}
local time_ms = 0
for i, v in pairs(steps) do
	local data = {
		label = Textlabel.new();
	}

	data.label.font = font
	data.label.text = v
	data.show_time = i * 2000
	data.show_time_finish = data.show_time + 2000
	data.label.position = Vec2.new(0, 64 * (i - 1))

	state[i] = data
end

-- local objects = {}
-- local paths = {
-- 	{ "image/fox.jpg",   "mesh/box.obj" },
-- 	{ "image/eotek.png", "mesh/sphere.obj" },
-- }

-- for i, v in pairs(paths) do
-- 	warn("Loading object", v[1], v[2])
-- 	local texture = _temp_Image()
-- 	texture:load(v[1])

-- 	local mesh = _temp_Mesh()
-- 	mesh:load(v[2])
-- 	mesh.texture = texture

-- 	objects[i] = { texture, mesh }
-- end

-- local next_check = now() + 1
-- local fps = 0
-- local i = 0

-- local base_matrix = glm.mat4(1)
-- camera.position = base_matrix

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)
	time_ms = time_ms + (delta_time * 1000)

	for i, v in pairs(state) do
		local alpha = math.max(math.min(map(time_ms, v.show_time, v.show_time_finish, 0, 1), 1), 0)
		v.label.scale = alpha * 0.5
		v.label.color = Vec3.new(1, 1, math.abs(math.sin(now() / 5000)))
		v.label:render()
	end

-- 	local to_render = objects[(i % #objects) + 1]

-- 	local pos = glm.vec3(math.sin(now() * 0.001) * 0.5, 0, -1)
-- 	camera.position = base_matrix:mat4_translate_vec3(pos)

-- 	to_render[2]:render()

-- 	if now() > next_check then
-- 		textlabel.text = string.format("memory: %.2f kb", collectgarbage("count"))

-- 		next_check = now() + 2000
-- 		fps = 0

-- 		i = i + 1
-- 	end

end)

-- print("Lua entry file finished")
