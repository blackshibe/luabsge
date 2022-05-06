---@diagnostic disable: undefined-global

-- custom libraries...
require("SomeCustomLibrary")
Module.somecustomlibrary.lorem()

local font = Module.assetloader.load_font("font/FiraCode-Regular.ttf")
local textlabel = CreateObject(font)

local texture = Module.assetloader.load_image("image/fox.png")
local mesh = World.meshloader.load_mesh("mesh/box.obj")
local box = CreateObject(mesh, texture)

World.rendering.step:connect(function(delta_time)
	textlabel:GetComponentOfType("Text").text = tostring(delta_time * 60)
	box.transform = Transform.new(Vector3.new(0, 10, 0))
end)
