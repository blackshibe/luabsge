---@diagnostic disable: undefined-global

require("assets")

local spring = require("script.spring")
local max_axis_resize = require("script.max_axis_resize")

local glitch_effect = VFXEffect.new()
glitch_effect:load_fragment_shader("shader/glitch_frag.glsl")

local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100
camera.transform = Mat4.new(1)

local targetSceneBufferSize = Vec2.new(2000, 1333)
local scene_buffer = Framebuffer.new(targetSceneBufferSize.x, targetSceneBufferSize.y)

local font = Font.new()
font:load("font/arial.ttf")

local message = "blackshibe.net"
local display_label = Textlabel.new()
display_label.font = font

local camera_x_spring = spring.new(0.5, 100, 50)
local camera_y_spring = spring.new(0.5, 100, 50)

function render_pass()
	display_label:render()
	World.rendering.render_pass()
end

-- TODO mouse position becomes nan
-- TODO clean up code

local use_shader = true
local last_mouse_position = World.input.get_mouse_position()
local next_update = (1 / 30) * 1000
local start = now()
local render_other_scene = false

Window.set_vsync(false)
Window.maximize()

World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)
	local mouse_position = World.input.get_mouse_position()
	local mouse_delta = mouse_position - last_mouse_position
	last_mouse_position = mouse_position

	local mouse_position_scaled = last_mouse_position / Window.get_window_dimensions()
	camera_x_spring.target = mouse_position_scaled.x
	camera_y_spring.target = mouse_position_scaled.y

	local camera_x = camera_x_spring:update(delta_time)
	local camera_y = camera_y_spring:update(delta_time)

	if render_other_scene then
		COMMON_PATH = ""
		require("other-scene")(delta_time)
	else
		camera.transform = Mat4.new(1):translate(Vec3.new(-camera_x + 0.5, camera_y - 0.5, -4) * 0.25)
	end

	display_label.position = Vec2.new(100, Window.get_window_dimensions().y - 100)
	display_label.anchor = Vec2.new(0, 1)
	display_label.scale = 1.0
	display_label.text = message:sub(1, math.floor(now() / 200) % (#message + 1))

	local glitch_factor = 0.01 + (mouse_delta.x + mouse_delta.y) / 500
	if use_shader and glitch_effect.is_valid then
		if now() > next_update then
			next_update = now() + (1 / 15) * 1000

			scene_buffer:resize(max_axis_resize(Window.get_window_dimensions(), targetSceneBufferSize))
			scene_buffer:clear()

			scene_buffer:bind(function()
				render_pass()
			end)

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
		ImGui.Text(string.format("Mouse position: %.1f, %.1f", camera_x, camera_y))
		ImGui.Text(string.format("Delta: %.2f", glitch_factor))
		ImGui.Text(string.format("FPS: %.2f", 1 / delta_time))

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
