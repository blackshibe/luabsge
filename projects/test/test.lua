warn("running tests")

local expected_members = {
	"rendering",
	"ui",
	"meshloader",
}

for i, v in pairs(expected_members) do
	assert(World[v], string.format("%s is not loaded inside World", v))
end

local font = World.ui.load_font("font/FiraCode-Regular.ttf")
assert(getmetatable(font), "font has no metatable attached")
