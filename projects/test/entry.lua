---@diagnostic disable: undefined-global

local font = Font.new()
font:load(COMMON_PATH .. "font/FiraCode-Regular.ttf")

local display_label = Textlabel.new()
display_label.font = font

-- TODO: can't use separate cameras for separate buffers(?)
local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

-- TODO: mesh textures are not optional
local texture = Image.new()
texture:load(COMMON_PATH .. "image/fox.jpg")

-- TODO: mesh and physicsobject should not be separate in this way
-- TODO: mesh scale needs to be reapplied when getting physics object translation every frame
local mesh = Mesh.new()
mesh:load(COMMON_PATH .. "mesh/box.obj")
mesh.texture = texture
mesh.matrix = Mat4.new(1):translate(Vec3.new(0, 1.5, 0))
local phys_object = PhysicsObject.new(mesh, true)

local mesh_top = Mesh.new()
mesh_top:load(COMMON_PATH .. "mesh/box.obj")
mesh_top.texture = texture
mesh_top.matrix = Mat4.new(1):translate(Vec3.new(1, 4, 0.25))
local phys_object_top = PhysicsObject.new(mesh_top, true)

local floor = Mesh.new()
floor:load(COMMON_PATH .. "mesh/box.obj")
floor.texture = texture
floor.matrix = Mat4.new(1):scale(Vec3.new(10, 0.1, 10))
local floor_phys = PhysicsObject.new(floor, false)

local raytracer_framebuffer = Framebuffer.new(256, 256)

local camera_z_spring = require("script.spring").new(0)
local base_matrix = Mat4.new(1)

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
camera.matrix = base_matrix

function render_pass()
	-- TODO should not be necessary
	mesh:render()
	floor:render()
	mesh_top:render()
	display_label:render()

	Gizmo.set_line_width(0.05)
	Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))

	Gizmo.set_line_width(2)
	Gizmo.draw_line(Vec3.new(), Vec3.new(10, 0, 0), Vec3.new(1, 0, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 10, 0), Vec3.new(0, 1, 0))
	Gizmo.draw_line(Vec3.new(), Vec3.new(0, 0, 10), Vec3.new(0, 0, 1))
end

World.rendering.step:connect(function(delta_time)
	local dim = Window.get_window_dimensions()
	local alpha = math.sin(now() * 0.001) * 0.5
	display_label.position = Vec2.new(dim.x / 2, dim.y / 2)
	display_label.anchor = Vec2.new(0.5, alpha)
	display_label.scale = 0.5 + alpha
	display_label.color = Vec3.new(1, alpha, math.abs(math.sin(now() / 5000)))

	local camera_z = camera_z_spring:update(delta_time)

	-- TODO camera matrix is only sent to the shader once per frame, and afaik right here it's already out of date by 1 frame
	-- it would make sense to have a primary window framebuffer object that's assigned in World instead
	camera.matrix = Mat4.new(1)
		:translate(Vec3.new(0, 0, -10))
		:rotate(0.25, Vec3.new(1, 0, 0))
		:rotate((now() / 5000) % math.pi * 2, Vec3.new(0, 0.5, 0))

	-- TODO should not be necessary
	mesh.matrix = phys_object:get_transform()
	mesh_top.matrix = phys_object_top:get_transform()
	floor.matrix = floor_phys:get_transform():scale(Vec3.new(10, 0.1, 10))

	raytracer_framebuffer:clear()

	-- TODO bind(function() end) instead
	raytracer_framebuffer:bind()

	render_pass()
	raytracer_framebuffer:unbind()

	render_pass()

	if ImGui.Begin("LuaBSGE ImGui Demo") then
		ImGui.Image(raytracer_framebuffer.texture_id, Vec2.new(800, 600))

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
