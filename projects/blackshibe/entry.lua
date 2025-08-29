---@diagnostic disable: undefined-global

local spring = require("script.spring")

local glitch_effect = VFXEffect.new()
glitch_effect:load_fragment_shader("shader/glitch_frag.glsl")

local scene_buffer = Framebuffer.new(1024, 1024)

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local texture = Image.new()
texture:load("image/fox.jpg")

local mesh = Mesh.new()
mesh:load("mesh/plane.obj")
mesh.texture = texture

local camera_x_spring = spring.new(0.5, 10, 10)
local camera_y_spring = spring.new(0.5, 10, 10)
local base_matrix = Mat4.new(1)

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
camera.matrix = base_matrix

function render_pass()
	mesh:render()
end

local use_shader = true
local buffer_dimens_x = 512
local buffer_dimens_y = 512
World.rendering.step:connect(function(delta_time)
	Gizmo.set_line_width(0.05)
	Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))

	Gizmo.set_line_width(2)
	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))

	-- TODO mouse position doesn't match up with real position
	local mouse_pos = World.input.get_mouse_position() / Window.get_window_dimensions()
	-- camera_x_spring.target = mouse_pos.x
	-- camera_y_spring.target = mouse_pos.y

	local camera_x = camera_x_spring:update(delta_time)
	local camera_y = camera_y_spring:update(delta_time)

	mesh.matrix = Mat4.new(1):rotate(math.pi / 2, Vec3.new(1, 0, 0))
	camera.matrix = Mat4.new(1):translate(Vec3.new(-camera_x + 0.5, camera_y - 0.5, -5))
	-- Mat4.new(1):translate(Vec3.new(0, 0, -10)):rotate(0.1, Vec3.new(0, 1, 0)):rotate(0.1, Vec3.new(1, 0, 0))
	-- :translate(Vec3.new(camera_y / 1000, camera_y / 1000, -5))

	if use_shader then
		scene_buffer:resize(Vec2.new(buffer_dimens_x, buffer_dimens_y))
		scene_buffer:clear()

		scene_buffer:bind()
		render_pass()
		scene_buffer:unbind()

		scene_buffer:bind_texture(GL_TEXTURE0)
		scene_buffer:bind_texture(GL_TEXTURE1)

		glitch_effect:set_uniform_int("u_image", 0) -- texture 0
		glitch_effect:set_uniform_int("u_inverted_texture", 1) -- texture 1

		glitch_effect:set_uniform_float("u_glitch", 10.0)
		glitch_effect:set_uniform_float("u_invert", 0)
		glitch_effect:set_uniform_vec2("u_mouse_position", 0, 0)

		glitch_effect:render()
	else
		render_pass()
	end

	if ImGui.Begin("LuaBSGE ImGui Demo") then
		ImGui.Image(texture.id, Vec2.new(128, 128))
		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()
		ImGui.Text(string.format("Mouse position: %.1f, %.1f", camera_x, camera_y))

		_, use_shader = ImGui.Checkbox("Use shader?", use_shader)
		_, buffer_dimens_x = ImGui.SliderInt("Res X", buffer_dimens_x, 1, math.floor(Window.get_window_dimensions().x))
		_, buffer_dimens_y = ImGui.SliderInt("Res Y", buffer_dimens_y, 1, math.floor(Window.get_window_dimensions().y))
	end
	ImGui.End()
end)

print("Lua entry file finished")
