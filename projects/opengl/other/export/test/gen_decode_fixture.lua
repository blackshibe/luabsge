-- Builds a self-contained test/decode_in/ directory that mimics what
-- download.lua leaves on disk, so decode.lua can be tested without ever
-- hitting the network:
--   test/decode_in/mesh/100.mesh           v2 mesh, one triangle
--   test/decode_in/animation/200.rbxm      KeyframeSequence with 2 keyframes
--   test/decode_in/manifest.json           entries pointing at both
local fs     = require("@lune/fs")
local serde  = require("@lune/serde")
local roblox = require("@lune/roblox")

local root = "test/decode_in"
if fs.isDir(root) then fs.removeDir(root) end
fs.writeDir(root)
fs.writeDir(root .. "/mesh")
fs.writeDir(root .. "/animation")

-- ---------------------------------------------------------------------------
-- Synthetic v2 mesh: header + 3 verts + 1 face. Vertex stride 40 = 32 (pos +
-- nrm + uv) + 4 (tangent sbyte4) + 4 (rgba).
-- ---------------------------------------------------------------------------

local mesh = "version 2.00\n"
mesh = mesh .. string.pack("<HBBII", 12, 40, 12, 3, 1)

local function vert(px, py, pz, nx, ny, nz, tu, tv)
    return string.pack("<ffffffff", px, py, pz, nx, ny, nz, tu, tv)
        .. "\0\0\0\0"          -- tangent
        .. "\255\255\255\255"  -- rgba
end
mesh = mesh .. vert(0, 0, 0,  0, 0, 1,  0, 0)
mesh = mesh .. vert(1, 0, 0,  0, 0, 1,  1, 0)
mesh = mesh .. vert(0, 1, 0,  0, 0, 1,  0, 1)
mesh = mesh .. string.pack("<III", 0, 1, 2)

fs.writeFile(root .. "/mesh/100.mesh", mesh)
print(string.format("wrote %s/mesh/100.mesh (%d bytes)", root, #mesh))

-- ---------------------------------------------------------------------------
-- KeyframeSequence with two keyframes, each carrying a small Pose tree:
--   HumanoidRootPart (root)
--     Torso
-- ---------------------------------------------------------------------------

local function pose(name, cf)
    local p = roblox.Instance.new("Pose")
    p.Name   = name
    p.CFrame = cf
    return p
end

local seq = roblox.Instance.new("KeyframeSequence")
seq.Name = "TestAnim"
seq.Loop = true

for i, t in ipairs({ 0.0, 0.5 }) do
    local kf = roblox.Instance.new("Keyframe")
    kf.Time = t
    kf.Name = "Frame" .. i
    kf.Parent = seq

    local root_p = pose("HumanoidRootPart", roblox.CFrame.new(0, t, 0))
    root_p.Parent = kf

    local torso = pose("Torso", roblox.CFrame.new(0, 1, 0))
    torso.Parent = root_p
end

fs.writeFile(root .. "/animation/200.rbxm", roblox.serializeModel({ seq }))
print(string.format("wrote %s/animation/200.rbxm", root))

-- ---------------------------------------------------------------------------
-- Manifest mimicking the download.lua output shape.
-- ---------------------------------------------------------------------------

local manifest = {
    assets = {
        ["100"] = {
            kind = "mesh",
            file = "mesh/100.mesh",
            status = "ok",
            bytes = #mesh,
            refs = { { instance = "Workspace/Test", property = "MeshContent" } },
        },
        ["200"] = {
            kind = "animation",
            file = "animation/200.rbxm",
            status = "ok",
            refs = { { instance = "Workspace/Test/Animation", property = "AnimationId" } },
        },
    },
}

fs.writeFile(root .. "/manifest.json", serde.encode("json", manifest, true))
print("wrote " .. root .. "/manifest.json")
