---@diagnostic disable: undefined-global

local font = Font.new()
font:load(COMMON_PATH .. "font/arial.ttf")

local display_label = Textlabel.new()
display_label.font = font

-- TODO: can't use separate cameras for separate buffers(?)
local camera = Camera.new()
camera.fov = 70 -- degrees
camera.near_clip = 0.1
camera.far_clip = 100

local boxes = {}
local next_spawn_time = now()

local box_mesh = Mesh.new(COMMON_PATH .. "mesh/box.obj")
local texture = Image.new(COMMON_PATH .. "image/fox.jpg")

local floor_object = Object.new()
floor_object.transform = Mat4.new(1):scale(Vec3.new(10, 0.1, 10))
floor_object:add_component(ECS_MESH_COMPONENT, { mesh = box_mesh })
floor_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })
floor_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box_mesh, is_dynamic = false })

local raytracer_framebuffer = Framebuffer.new(256, 256)

-- you must create a central camera yourself to define the default position of it
World.rendering.camera = camera
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

local camera_inputs = {
	r_x = 1.9,
	r_y = 0.1,
	pos_x = 0,
	pos_y = 0,
	pos_z = -5,
}

local MOUSE_SENSITIVITY = 0.002

-- TODO plane object isn't deleted when scene switches
-- TODO last new box has invalid mesh data

local function spawn_one()
	local box_object = Object.new()
	box_object.transform = Mat4.new(1)
		:scale(Vec3.new(0.5, 0.5, 0.5))
		:translate(Vec3.new(math.random(-100, 100) / 10, 15, math.random(-100, 100) / 10))

	box_object:add_component(ECS_MESH_COMPONENT, { mesh = box_mesh, color = Vec4.new(1, 1, 1, 0.5) })
	box_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })
	box_object:add_component(ECS_PHYSICS_COMPONENT, { mesh = box_mesh, is_dynamic = true })

	table.insert(boxes, box_object)
end

return function(delta_time)
	local dim = Window.get_window_dimensions()
	local alpha = math.sin(now() * 0.001) * 0.5
	display_label.position = Vec2.new(dim.x / 2, dim.y / 2)
	display_label.anchor = Vec2.new(0.5, alpha)
	display_label.scale = 0.5 + alpha
	display_label.color = Vec3.new(1, alpha, math.abs(math.sin(now() / 5000)))

	if now() > next_spawn_time then
		next_spawn_time = now() + 500

		spawn_one()
	end

	if World.input.is_left_mouse_down() then
		local delta = World.input.get_mouse_delta()
		camera_inputs.r_x = camera_inputs.r_x + delta.x * MOUSE_SENSITIVITY
		camera_inputs.r_y = camera_inputs.r_y + delta.y * MOUSE_SENSITIVITY

		raytracer_framebuffer:clear()
		accum_frames = 0
	end

	--  it would make sense to have a primary window framebuffer object that's assigned in World instead
	camera.transform = Mat4.new(1)
		:translate(Vec3.new(0, 0, -16))
		:rotate(camera_inputs.r_y, Vec3.new(1, 0, 0))
		:rotate(camera_inputs.r_x, Vec3.new(0, 1, 0))

	-- todo doesn't work idk why
	raytracer_framebuffer:clear()
	raytracer_framebuffer:bind(render_pass)

	if ImGui.Begin("LuaBSGE ImGui Demo") then
		ImGui.Image(raytracer_framebuffer.texture_id, Vec2.new(256, 256), false)
		ImGui.Text("Hold LMB and move the mouse to change the camera position")
		ImGui.Text(string.format("Objects: %s", #boxes))

		if ImGui.Button("Ok but give more") then
			for i = 1, 10 do
				spawn_one()
			end
		end

		ImGui.End()
	end
end
