require("test")

-- todo: actually use the string provided
local font = World.ui.load_font("font/FiraCode-Regular.ttf")

-- todo: implement
font.position = Vector2.new(10, 10)
-- meshloader needs to interface with Assimp much better as an .obj might not be a single mesh and can have different types of textures attached
local texture = World.ui.load_image("image/fox.jpg")
local mesh = World.meshloader.load_mesh("mesh/box.obj")

-- todo: implement
mesh:use_texture(texture)

-- todo: implement
-- local position = Vector3.new(10, 10, 0)
-- local color = Color.new(1, 1, 1)

World.rendering.step:connect(function(delta_time)
	-- font.scale = math.sin(now() * 5)
	World.rendering.draw_font("fox cube fox cube fox cube fox cube fox cube fox cube", font)
	-- todo: remove
	-- use_texture(texture)

	-- todo: remove (replace with some kind of world structure)
	-- mesh would be put inside the root as box.obj or something
	render(mesh)
end)
