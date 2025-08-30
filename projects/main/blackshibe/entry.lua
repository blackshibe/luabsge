---@diagnostic disable: undefined-global

local spring = require("script.spring")

local glitch_effect = VFXEffect.new()
glitch_effect:load_fragment_shader("shader/glitch_frag.glsl")

local scene_buffer = Framebuffer.new(2048, 1333)

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local texture = Image.new(string.format("image/active-%s.jpg", math.random(1, 5)))

local mesh = Mesh.new()
mesh:load("mesh/plane.obj")
mesh.texture = texture

local font = Font.new()
font:load("font/arial.ttf")

local message = "blackshibe.net"
local display_label = Textlabel.new()
display_label.font = font

local camera_x_spring = spring.new(0.5, 100, 50)
local camera_y_spring = spring.new(0.5, 100, 50)
local base_matrix = Mat4.new(1)

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
camera.matrix = base_matrix

function render_pass()
	display_label:render()
	mesh:render()
end

-- TODO: buffers don't work in librewolf, so images are at 1k res rn (<1k max texture res)
-- TODO: blackshibe.net interactivity (listening to hover events and stuff)

local use_shader = true
local last_mouse_position = World.input.get_mouse_position()
local next_update = (1 / 30) * 1000
local start = now()

local render_other_scene = false
World.rendering.step:connect(function(delta_time)
	if render_other_scene then
		COMMON_PATH = ""
		require("other-scene")(delta_time)

		return
	end
	-- Gizmo.set_line_width(0.05)
	-- Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))

	-- Gizmo.set_line_width(2)
	-- Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	-- Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	-- Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))

	display_label.position = Vec2.new(100, Window.get_window_dimensions().y - 100)
	display_label.anchor = Vec2.new(0, 1)
	display_label.scale = 1.0
	display_label.text = message:sub(1, math.floor(now() / 200) % (#message + 1))

	-- TODO mouse position doesn't match up with real position
	local mouse_position = World.input.get_mouse_position()
	local mouse_delta = mouse_position - last_mouse_position
	last_mouse_position = mouse_position

	local mouse_position_scaled = last_mouse_position / Window.get_window_dimensions()
	camera_x_spring.target = mouse_position_scaled.x
	camera_y_spring.target = mouse_position_scaled.y

	local camera_x = camera_x_spring:update(delta_time)
	local camera_y = camera_y_spring:update(delta_time)

	mesh.matrix =
		Mat4.new(1):scale(Vec3.new(texture.width / texture.height, 1, 1)):rotate(math.pi / 2, Vec3.new(1, 0, 0))
	camera.matrix = Mat4.new(1):translate(Vec3.new(-camera_x + 0.5, camera_y - 0.5, -4) * 0.25)
	-- Mat4.new(1):translate(Vec3.new(0, 0, -10)):rotate(0.1, Vec3.new(0, 1, 0)):rotate(0.1, Vec3.new(1, 0, 0))
	-- :translate(Vec3.new(camera_y / 1000, camera_y / 1000, -5))

	local glitch_factor = 0.01 + (mouse_delta.x + mouse_delta.y) / 500
	if use_shader and glitch_effect.is_valid then
		if now() > next_update then
			next_update = now() + (1 / 15) * 1000

			-- scene_buffer:resize(Vec2.new(Window.get_window_dimensions().x, Window.get_window_dimensions().y))
			scene_buffer:clear()

			scene_buffer:bind()
			render_pass()
			scene_buffer:unbind()

			scene_buffer:bind_texture(GL_TEXTURE0)
			scene_buffer:bind_texture(GL_TEXTURE1)

			glitch_effect:set_uniform_float("u_time", now() - start) -- texture 0
			glitch_effect:set_uniform_int("u_image", 0) -- texture 0
			glitch_effect:set_uniform_int("u_inverted_texture", 1) -- texture 1

			glitch_effect:set_uniform_float("u_glitch", glitch_factor)
			glitch_effect:set_uniform_float("u_invert", 0)
			glitch_effect:set_uniform_vec2("u_mouse_position", mouse_position_scaled.x, 1 - mouse_position_scaled.y)
		end

		glitch_effect:render()
	else
		render_pass()
	end

	if ImGui.Begin("Look at me I'm a fucking C++ application") then
		ImGui.Text("Image")
		ImGui.Image(texture.id, Vec2.new(texture.width * 0.1, texture.height * 0.1), true)
		ImGui.Spacing()
		ImGui.Separator()

		ImGui.Text(string.format("Mouse position: %.1f, %.1f", camera_x, camera_y))
		ImGui.Text(string.format("Delta: %.2f", glitch_factor))

		ImGui.Spacing()
		ImGui.Separator()

		_, use_shader = ImGui.Checkbox("Use shader?", use_shader)

		local pressed = ImGui.Button("Switch scene")
		if pressed then
			render_other_scene = true
		end

		ImGui.End()
	end
end)

print("Lua entry file finished")
