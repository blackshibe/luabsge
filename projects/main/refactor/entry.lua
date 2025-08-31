---@diagnostic disable: undefined-global, redundant-parameter, undefined-field, inject-field, assign-type-mismatch

local camera = Camera.new()
camera.fov = 70
camera.near_clip = 0.1
camera.far_clip = 100

-- TODO: these should be userdatas that store information only
local texture = Image.new(COMMON_PATH .. "image/fox.jpg")
local mesh = Mesh.new(COMMON_PATH .. "mesh/box.obj")
mesh.texture = texture -- legacy

local function part(transform, parent)
	local object = Object.new()
	object.transform = transform
	object.parent = parent

	object:add_component(ECS_MESH_COMPONENT, { mesh = mesh })
	object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture }) -- later mesh material

	return object
end

local box = part(Mat4.new(1), Scene)
local box_parent = part(Mat4.new(1):translate(Vec3.new(0, 0, 2)), box)
local box_parent_2 = part(Mat4.new(1):translate(Vec3.new(0, 0, 2)), box_parent)

World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)
	camera.transform = Mat4.new(1)
		:translate(Vec3.new(0, 0, -10))
		:rotate(0.25, Vec3.new(1, 0, 0))
		:rotate((now() / 5000) % math.pi * 2, Vec3.new(0, 0.5, 0))

	Gizmo.set_line_width(0.05)
	Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))

	local x = (now() / 1000) % math.pi * 2
	box.transform = Mat4.new(1):rotate(x, Vec3.new(0, 1, 0))
	box_parent.transform = Mat4.new(1):translate(Vec3.new(0, 0, 2)):rotate(x, Vec3.new(0, 0, 1))
	box_parent_2.transform = Mat4.new(1):translate(Vec3.new(0, 0, 2)):rotate(x, Vec3.new(0, 0, 1))

	World.rendering.render_pass()
end)
