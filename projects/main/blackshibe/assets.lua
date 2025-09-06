local plane_mesh = Mesh.new("mesh/plane.obj")
local function loadImageAsMesh(texture)
	local plane_object = Object.new()
	plane_object.transform =
		Mat4.new(1):scale(Vec3.new(texture.width / texture.height, 1, 1)):rotate(math.pi / 2, Vec3.new(1, 0, 0))
	plane_object.parent = Scene

	-- TODO throw error when setting no mesh
	plane_object:add_component(ECS_MESH_COMPONENT, { mesh = plane_mesh })
	plane_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })

	return plane_object
end

SCENE_TEXTURES = {
	backdrop = PLATFORM == "NATIVE" and Image.new(string.format("image/active-%s.jpg", math.random(1, 5)))
		or Emscripten.download_image("image.jpg"),
}

SCENE_OBJECTS = {
	backdrop = loadImageAsMesh(SCENE_TEXTURES.backdrop),
}
