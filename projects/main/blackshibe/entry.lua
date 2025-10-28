---@diagnostic disable: undefined-global

-- TODO mouse position becomes nan
-- TODO clean up code

require("assets")
MAX_BUFFER_RESOLUTION = Vec2.new(2000, 1333)

local spring = require("script.spring")
local max_axis_resize = require("script.max_axis_resize")

local glitch_effect = VFXEffect.new()
glitch_effect:load_fragment_shader("shader/glitch_frag.glsl")

local scene_buffer = Framebuffer.new(MAX_BUFFER_RESOLUTION.x, MAX_BUFFER_RESOLUTION.y)

local camera = Camera.new()
camera.fov = 70
camera.near_clip = 0.1
camera.far_clip = 100

local font = Font.new()
font:load("font/arial.ttf")

local display_label = Textlabel.new()
display_label.font = font

local camera_x_spring = spring.new(0.5, 100, 50)
local camera_y_spring = spring.new(0.5, 100, 50)

local use_shader = true
local last_mouse_position = World.input.get_mouse_position()
local next_update = (1 / 30) * 1000
local start = now()

local last_browser_text = 0
local last_browser_text_expiry = now()
local view_box_factor = math.random(0, 1)

local motive_text_index = math.random(1, #MOTIVE_TEXTS)
local render_other_scene = false

if PLATFORM == "NATIVE" then
	Window.set_vsync(false)
	Window.maximize()
end

camera.transform = Mat4.new(1)
World.rendering.camera = camera

function render_pass()
	display_label:render()
	World.rendering.render_pass()
end

World.rendering.step:connect(function(delta_time)
	if render_other_scene then
		COMMON_PATH = ""
		require("other-scene")(delta_time)
	end

	-- TODO mouse position doesn't match up with real position
	local mouse_position = World.input.get_mouse_position()
	local mouse_delta = mouse_position - last_mouse_position
	last_mouse_position = mouse_position

	local mouse_position_scaled = last_mouse_position / Window.get_window_dimensions()
	camera_x_spring.target = mouse_position_scaled.x
	camera_y_spring.target = mouse_position_scaled.y

	local camera_x = math.max(math.min(camera_x_spring:update(delta_time), 1), -1)
	local camera_y = math.max(math.min(camera_y_spring:update(delta_time), 1), -1)

	camera.transform = Mat4.new(1):translate(Vec3.new(-camera_x + 0.5, camera_y - 0.5, -4) * 0.25)

	-- update browser text
	do
		local default = MOTIVE_TEXTS[motive_text_index]
		local cur_message = BROWSER_TEXTS[last_browser_text + 1] or default
		local completion_factor = 0
		display_label.scale = 0.6

		if now() > last_browser_text_expiry then
			display_label.position = Vec2.new(100, Window.get_window_dimensions().y - 100)
			display_label.anchor = Vec2.new(0, 1)

			cur_message = default
			completion_factor = math.floor(now() / 200) % ((#default + 1) * 3)
		else
			display_label.position = Vec2.new(
				Window.get_window_dimensions().x - 100,
				(0.2 + last_browser_text * 0.07) * Window.get_window_dimensions().y
			)
			display_label.anchor = Vec2.new(1, 0.5)

			completion_factor = #cur_message
		end

		display_label.text = cur_message:sub(1, completion_factor)

		if Emscripten.get_int("selected") ~= last_browser_text then
			last_browser_text = Emscripten.get_int("selected")
			last_browser_text_expiry = now() + 6000
		end
	end

	-- update box
	do
		local spin_factor = (1 - view_box_factor)
		local pos_active = Mat4.new(1):rotate((now() * 0.0005) % math.pi * 2 * spin_factor, Vec3.new(1, 0.5, 0))

		local texture = spin_factor == 1 and SCENE_TEXTURES.fox or SCENE_TEXTURES.active_image
		SCENE_OBJECTS.main_box.transform = Mat4.new(1)
			:scale(Vec3.new(texture.width / texture.height, 1, 1))
			:translate(Vec3.new(0, 0, -1 - spin_factor * 3)) * pos_active
		SCENE_OBJECTS.main_box:patch_component(
			ECS_MESH_TEXTURE_COMPONENT,
			{ texture = spin_factor == 1 and SCENE_TEXTURES.fox or SCENE_TEXTURES.active_image }
		)
	end

	-- update shader
	local glitch_factor = 0.01 + (mouse_delta.x + mouse_delta.y) / 500
	local framerate = render_other_scene and 60 or 15
	if use_shader and glitch_effect.is_valid then
		if now() > next_update then
			next_update = now() + (1 / framerate) * 1000

			scene_buffer:resize(max_axis_resize(Window.get_window_dimensions(), MAX_BUFFER_RESOLUTION))
			scene_buffer:bind(function()
				render_pass()
			end)

			scene_buffer:bind_texture(GL_TEXTURE4)

			glitch_effect:set_uniform_float("u_time", now() - start) -- texture 0
			glitch_effect:set_uniform_int("u_image", 4) -- texture 0
			glitch_effect:set_uniform_int("u_inverted_texture", 4) -- texture 1

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
		ImGui.Text(string.format("Emscripten test int: %i", Emscripten.get_int("selected")))
		-- doesn't work lmao
		if PLATFORM ~= "WEB" then
			ImGui.Text(string.format("FPS: %.2f", 1 / delta_time))
		end
		ImGui.Text(string.format("Viewing: %s", view_box_factor == 1 and "true" or "false"))

		if not render_other_scene then
			ImGui.Spacing()
			ImGui.Separator()

			_, use_shader = ImGui.Checkbox("Use shader?", use_shader)

			local pressed = ImGui.Button(view_box_factor == 1 and "Show box" or "View photo")
			if pressed then
				view_box_factor = 1 - view_box_factor
			end

			local pressed_scene = ImGui.Button("Switch scene")
			if pressed_scene then
				render_other_scene = true
			end
		end

		ImGui.End()
	end
end)

print("Lua entry file finished")
