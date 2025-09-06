local plane_mesh = Mesh.new("mesh/plane.obj")
local function loadImageAsMesh(texture, opacity)
	local plane_object = Object.new()
	plane_object.transform = Mat4.new(1):scale(Vec3.new(texture.width / texture.height, 1, 1))
	plane_object.parent = Scene

	plane_object:add_component(ECS_MESH_COMPONENT, { mesh = plane_mesh, color = Vec4.new(1, 1, 1, opacity) })
	plane_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })

	return plane_object
end

SCENE_TEXTURES = {
	-- backdrop = PLATFORM == "NATIVE" and Image.new(string.format("image/active-%s.jpg", math.random(1, 5)))
	-- 	or Emscripten.download_image("image.jpg"),
	backdrop = Image.new(string.format("image/active-%s.jpg", math.random(1, 5))),
	frame = Image.new("image/ui/black.png"),
}

SCENE_OBJECTS = {
	backdrop = loadImageAsMesh(SCENE_TEXTURES.backdrop, 1),
	frame = loadImageAsMesh(SCENE_TEXTURES.frame, 0.9),
}

SCENE_OBJECTS.backdrop.transform = SCENE_OBJECTS.backdrop.transform:rotate(math.pi / 2, Vec3.new(1, 0, 0))
SCENE_OBJECTS.frame.transform = Mat4.new(1):rotate(math.pi / 2, Vec3.new(1, 0, 0)):translate(Vec3.new(1.5, 0.1, 0))
