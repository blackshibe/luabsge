	---@diagnostic disable: undefined-global

	-- I can't wait to not touch this project for another 6 months and never use this
	require("tester.loader")

	local font = _temp_Font()
	font:load("font/FiraCode-Regular.ttf")

	local textlabel = _temp_TextLabel()
	textlabel.font = font

	local camera = _temp_Camera()
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
		local texture = _temp_Image()
		texture:load(v[1])

		local mesh = _temp_Mesh()
		mesh:load(v[2])
		mesh.texture = texture

		objects[i] = { texture, mesh }
	end

	local next_check = now() + 1
	local fps = 0
	local i = 0

	local base_matrix = glm.mat4(1)
	camera.position = base_matrix

	-- you must create a central camera yourself to define the default position of it
	World.rendering.camera = camera
	World.rendering.step:connect(function(delta_time)
		fps = fps + 1

		local to_render = objects[(i % #objects) + 1]
		local pos = glm.vec3(math.sin(now() * 0.001) * 0.5, 0, -1)
		camera.position = base_matrix:mat4_translate_vec3(pos)
		to_render[2]:render()

		if now() > next_check then
			-- textlabel.text = string.format("fps: %.2f, memory: %.2f kb", fps, collectgarbage("count"))
			print(string.format("fps: %.2f, memory: %.2f kb", fps, collectgarbage("count")))
			warn("Reminder: rewrite all of this in Rust")

			next_check = now() + 1000
			fps = 0

			i = i + 1
		end

		textlabel:render()
	end)

	print("Lua entry file finished")
