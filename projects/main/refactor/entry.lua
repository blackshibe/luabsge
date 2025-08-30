---@diagnostic disable: undefined-global, redundant-parameter, undefined-field

local camera = Camera.new()
camera.fov = 70
camera.near_clip = 0.1
camera.far_clip = 100

-- TODO: these should be userdatas that store information only
local texture = Image.new("image/fox.jpg")
local mesh = Mesh.new("mesh/box.obj")

local object = Object.new()
object.transform = Mat4.new()
object.parent = World.scene

object:AddComponent(MeshComponent, { mesh = mesh })
object:AddComponent(MeshTextureComponent, { texture = texture }) -- later mesh material
object:AddComponent(PhysicsComponent, { is_dynamic = true })

World.rendering.camera = camera
World.rendering.step:connect(function(delta_time)
	camera.transform = Mat4.new(1)
		:translate(Vec3.new(0, 0, -10))
		:rotate(0.25, Vec3.new(1, 0, 0))
		:rotate((now() / 5000) % math.pi * 2, Vec3.new(0, 0.5, 0))

	Gizmo.set_line_width(0.05)
	Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))
end)
