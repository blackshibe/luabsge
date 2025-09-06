---@diagnostic disable: undefined-global

local font = Font.new()
font:load(COMMON_PATH .. "font/FiraCode-Regular.ttf")

local display_label = Textlabel.new()
display_label.font = font

local primary_camera = Camera.new()
primary_camera.fov = 90 -- degrees
primary_camera.near_clip = 0.1
primary_camera.far_clip = 100

local secondary_camera = Camera.new()
secondary_camera.fov = 40 -- degrees
secondary_camera.near_clip = 0.1
secondary_camera.far_clip = 100

local texture = Image.new(COMMON_PATH .. "image/fox.jpg")
local texture_grid = Image.new(COMMON_PATH .. "image/grid_04.png")
local box = Mesh.new(COMMON_PATH .. "mesh/box.obj")

local box_object = Object.new()
box_object.transform = Mat4.new(1):translate(Vec3.new(0, 4, 0))
box_object:add_component(ECS_MESH_COMPONENT, { mesh = box })
box_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })
box_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box, is_dynamic = true })

local top_object = Object.new()
top_object.transform = Mat4.new(1):translate(Vec3.new(1, 6, 0.25))
top_object:add_component(ECS_MESH_COMPONENT, { mesh = box, color = Vec4.new(1, 1, 1, 0.5) })
top_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })
top_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box, is_dynamic = true })

local floor_object = Object.new()
floor_object.transform = Mat4.new(1):scale(Vec3.new(4, 0.1, 4))
floor_object:add_component(ECS_MESH_COMPONENT, { mesh = box })
floor_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture_grid })
floor_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box, is_dynamic = false })

local framebuffer = Framebuffer.new(512, 512)
local base_matrix = Mat4.new(1)

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = primary_camera
primary_camera.transform = base_matrix

function render_pass()
	display_label:render()
	World.rendering.render_pass()

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

	primary_camera.transform = Mat4.new(1)
		:translate(Vec3.new(0, 0, -10))
		:rotate(0.25, Vec3.new(1, 0, 0))
		:rotate((now() / 5000) % math.pi * 2, Vec3.new(0, 0.5, 0))

	secondary_camera.transform =
		Mat4.new(1):translate(Vec3.new(0, 0, -10)):rotate(0.25, Vec3.new(1, 0, 0)):rotate(0.25, Vec3.new(0, 0.5, 0))

	framebuffer:clear()

	-- TODO bind(function() end) instead
	World.rendering.camera = secondary_camera
	framebuffer:bind(function()
		render_pass()
	end)

	World.rendering.camera = primary_camera
	render_pass()

	if ImGui.Begin("LuaBSGE ImGui Demo") then
		ImGui.Image(framebuffer.texture_id, Vec2.new(300, 300), false)

		ImGui.Text("FPS: " .. string.format("%.1f", 1000.0 / (delta_time * 1000)))
		ImGui.Separator()
	end
	ImGui.End()
end)

print("Lua entry file finished")
