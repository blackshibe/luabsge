local plane_mesh = Mesh.new("mesh/box.obj")
local function loadImageAsMesh(texture, opacity)
	local plane_object = Object.new()
	plane_object.transform = Mat4.new(1):scale(Vec3.new(texture.width / texture.height, 1, 1))
	plane_object.parent = Scene

	plane_object:add_component(ECS_MESH_COMPONENT, { mesh = plane_mesh, color = Vec4.new(1, 1, 1, opacity) })
	plane_object:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = texture })

	return plane_object
end

SCENE_TEXTURES = {
	-- main_box = PLATFORM == "NATIVE" and Image.new(string.format("image/active-%s.jpg", math.random(1, 5)))
	-- 	or Emscripten.download_image("image.jpg"),
	fox = Image.new("image/fox.jpg"),
	active_image = Image.new(string.format("image/active-%s.jpg", math.random(1, 5))),
	frame = Image.new("image/ui/black.png"),
}

SCENE_OBJECTS = {
	main_box = loadImageAsMesh(SCENE_TEXTURES.active_image, 1),
	frame = loadImageAsMesh(SCENE_TEXTURES.frame, 1),
}

SCENE_OBJECTS.main_box.transform = SCENE_OBJECTS.main_box.transform:rotate(math.pi / 2, Vec3.new(1, 0, 0))
SCENE_OBJECTS.frame.transform =
	Mat4.new(1):translate(Vec3.new(1.5, 0.1, -45)):rotate(math.pi / 2, Vec3.new(0, 1, 0)):scale(Vec3.new(1, 100, 100))

BROWSER_TEXTS = {
	"email",
	"discord",
	"github",
	"bandcamp",
	"soundcloud",
	"youtube",
	"music",
	"photos",
	"friendly websites",
	"neofetch",
}

MOTIVE_TEXTS = {
	"blackshibe.net",
	-- "though the stars may shine",
	-- "we always think there is going to be more time",
	-- "watching the happiest days end",
}
