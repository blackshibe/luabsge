---@diagnostic disable: undefined-global

local meshes = {}
local texture = Image.new()
texture:load(COMMON_PATH .. "image/fox.jpg")

local function create_mesh(src)
	local mesh = Mesh.new()
	mesh:load(src)
	mesh.texture = texture

	return mesh
end

for i = 1, 32 do
	local obj2 = SphereBufferObject.new()
	obj2.center = Vec3.new(math.random(-10, 10), math.random(-10, 10), math.random(-10, 10))
	obj2.color = Vec3.new(math.random(0, 1), math.random(0, 1), math.random(0, 1))
	obj2.radius = math.random(1, 6) / 2
	obj2.emissive = 0
	obj2:register()
end

for i = 1, 12 do
	local obj2 = SphereBufferObject.new()
	obj2.center = Vec3.new(math.random(-10, 10), math.random(-10, 0), math.random(-10, 10))
	obj2.color = Vec3.new(math.random(0, 1), math.random(0, 1), math.random(0, 1))
	obj2.radius = math.random(1, 6) / 2
	obj2.emissive = math.random(30, 200)
	obj2:register()
end

local mesh = MeshBufferObject.new()
mesh.matrix = Mat4.new(1)
	:rotate(math.pi + 1, Vec3.new(0, 1, 0))
	:translate(Vec3.new(0, 0, -0.5))
	:scale(2)
	:rotate(0.1, Vec3.new(1, 0, 0))

mesh.color = Vec3.new(1, 0, 0)
mesh.emissive = 0
mesh.mesh = create_mesh(COMMON_PATH .. "mesh/suzanne.obj")
mesh:register()

return {
	meshes = {},
	monkey = mesh,
}
