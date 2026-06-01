-- Builds a synthetic .rbxl that exercises every case phase 2 cares about:
--   ServerScriptService/RootServer            (Script, leaf)
--   ServerScriptService/RootServer/Inner      (LocalScript inside a Script)
--   ReplicatedStorage/Lib                     (ModuleScript with children)
--   ReplicatedStorage/Lib/Helper              (ModuleScript, leaf)
--   ReplicatedStorage/Lib/data                (StringValue child of a script)
-- Source bodies include rbxassetid:// literals so phase 3's script-source
-- regex scan has something to find.
local fs     = require("@lune/fs")
local roblox = require("@lune/roblox")

local game = roblox.Instance.new("DataModel")

local function mk(class, name, parent, source)
	local inst = roblox.Instance.new(class)
	inst.Name = name
	inst.Parent = parent
	if source then inst.Source = source end
	return inst
end

local sss = game:GetService("ServerScriptService")
local rs  = game:GetService("ReplicatedStorage")

local root_server = mk("Script", "RootServer", sss, [[
-- Server entrypoint
print("hello from server")
local snd = Instance.new("Sound")
snd.SoundId = "rbxassetid://9120389081"
]])

mk("LocalScript", "Inner", root_server, [[
-- Nested LocalScript under a Script container.
local img = "rbxassetid://7059346373"
print("client inner", img)
]])

local lib = mk("ModuleScript", "Lib", rs, [[
-- Top-level module
local Lib = {}
Lib.icon = "rbxassetid://6764432408"
return Lib
]])

mk("ModuleScript", "Helper", lib, [[
return function() return "rbxassetid://6022668898" end
]])

mk("StringValue", "data", lib, nil) -- non-script child of a script container

fs.writeFile("test/fixture.rbxl", roblox.serializePlace(game))
print("wrote test/fixture.rbxl")
