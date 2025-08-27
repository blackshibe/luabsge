---@diagnostic disable: undefined-global

local meshes = {}
local texture = Image.new()
texture:load("image/fox.jpg")

local function create_mesh(src)
	local mesh = Mesh.new()
	mesh:load(src)
	mesh.texture = texture

	return mesh
end

local obj2 = SphereBufferObject.new()
obj2.center = Vec3.new(1, 0, -2)
obj2.color = Vec3.new(1, 1, 1)
obj2.radius = 1.5
obj2.emissive = 2
obj2:register()

local obj2 = SphereBufferObject.new()
obj2.center = Vec3.new(0, 0.5, 2)
obj2.color = Vec3.new(1, 0, 0)
obj2.radius = 0.5
obj2.emissive = 2
obj2:register()

local floor = MeshBufferObject.new()
local ceiling = MeshBufferObject.new()
table.insert(meshes, floor)
table.insert(meshes, ceiling)

floor.matrix = Mat4.new(1):translate(Vec3.new(0, -4, 0)):scale(Vec3.new(4, 0.1, 4))
floor.color = Vec3.new(0.5, 0.5, 0.5)
floor.emissive = 0
floor:register(create_mesh("mesh/box.obj"))

-- Set up second mesh (blue, static)
ceiling.matrix = Mat4.new(1):translate(Vec3.new(0, 4, 0)):scale(Vec3.new(4, 0.1, 4))
ceiling.color = Vec3.new(0.5, 0.5, 0.5)
ceiling.emissive = 0
ceiling:register(create_mesh("mesh/box.obj"))

for i = 3, 5 do
	local wall = MeshBufferObject.new()
	wall.matrix =
		Mat4.new(1):rotate(i * math.pi / 2, Vec3.new(0, 1, 0)):translate(Vec3.new(4, 0, 0)):scale(Vec3.new(0.1, 4, 4))
	wall.color = Vec3.new(1, 1, 1)

	if i == 3 then
		wall.color = Vec3.new(1, 0, 0)
	end
	if i == 5 then
		wall.color = Vec3.new(0, 1, 0)
	end

	wall.emissive = 0
	wall:register(create_mesh("mesh/box.obj"))

	table.insert(meshes, wall)
end

-- local mesh = MeshBufferObject.new()
-- mesh.matrix = Mat4.new(1)
-- mesh.color = Vec3.new(1, 0, 0)
-- mesh.emissive = 0
-- mesh:register(create_mesh("mesh/sphere.obj"))
-- table.insert(meshes, mesh)

return {
	meshes = meshes,
}
