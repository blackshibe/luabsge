-- Generates a runnable luabsge project from out/datamodel.json + manifest.json.
-- Picks up every MeshPart under Workspace with a decoded .obj on disk, copies
-- its mesh and texture into <target>, and emits config.lua + entry.lua.
--
--   lune run src/scene.lua [<out_dir>] [<target_project_dir>]
--
-- Default target is ../../scene, which resolves to <luabsge_root>/projects/scene
-- so you can run it directly:   bash run.sh scene   (or run_win.bat scene)

local fs = require("@lune/fs")
local serde = require("@lune/serde")
local process = require("@lune/process")
local util = require("./common/util")
local asset = require("./common/asset")
local manifest_lib = require("./common/manifest")

local args = process.args
local out_root = args[1] or "out"
local target = args[2] or "../../scene"

local datamodel = serde.decode("json", fs.readFile(out_root .. "/datamodel.json"))
local manifest, err = manifest_lib.read(out_root)
if not manifest then
	error(err)
end
local assets = manifest.assets or {}

local workspace = datamodel.children and datamodel.children.Workspace
if not workspace then
	error("Workspace not found in datamodel.json — was export.lua run?")
end

-- Returns the manifest-recorded path (relative to out_root) for an asset id,
-- but only if that path actually exists on disk.
local function asset_disk_path(id)
	local entry = assets[id]
	if entry and entry.file and fs.isFile(out_root .. "/" .. entry.file) then
		return entry.file
	end
	return nil
end

local parts = {}

local function visit(node, path)
	if node.class_name == "MeshPart" then
		local p = node.properties or {}
		local mesh_id = asset.id_from_property(p.MeshContent or p.MeshId)
		local mesh_file = mesh_id and asset_disk_path(mesh_id) or nil
		if mesh_file and mesh_file:find("%.obj$") and p.CFrame then
			local tex_id = asset.id_from_property(p.TextureContent or p.TextureID or p.TextureId)
			local tex_file = tex_id and asset_disk_path(tex_id) or nil
			table.insert(parts, {
				name = node.name,
				path = path,
				cframe = p.CFrame.components,
				size = p.Size, -- { __type, x, y, z } — used to scale the mesh to MeshPart.Size
				mesh_id = mesh_id,
				mesh_file = mesh_file,
				texture_id = tex_id,
				texture_file = tex_file,
			})
		end
	end
	for k, c in pairs(node.children or {}) do
		visit(c, path .. "/" .. k)
	end
end

visit(workspace, "Workspace")
print(string.format("found %d renderable MeshParts under Workspace", #parts))

-- Deterministic output across runs.
table.sort(parts, function(a, b)
	return a.path < b.path
end)

if fs.isDir(target) then
	fs.removeDir(target)
end
fs.writeDir(target)
fs.writeDir(target .. "/mesh")
fs.writeDir(target .. "/image")

local copied_meshes = {}
local copied_textures = {}

local function copy_into(subdir, src_rel, dest_name)
	local dest_rel = subdir .. "/" .. dest_name
	fs.writeFile(target .. "/" .. dest_rel, fs.readFile(out_root .. "/" .. src_rel))
	return dest_rel
end

for _, mp in ipairs(parts) do
	if not copied_meshes[mp.mesh_id] then
		copied_meshes[mp.mesh_id] = copy_into("mesh", mp.mesh_file, mp.mesh_id .. ".obj")
	end
	if mp.texture_file and not copied_textures[mp.texture_id] then
		local ext = mp.texture_file:match("%.([^.]+)$") or "png"
		copied_textures[mp.texture_id] = copy_into("image", mp.texture_file, mp.texture_id .. "." .. ext)
	end
end

print(string.format("copied %d meshes, %d textures into %s", util.count(copied_meshes), util.count(copied_textures), target))

-- Roblox renders MeshParts scaled to fill `Size`. The .mesh asset itself is in
-- its authored frame, so we need (Size / natural_bbox) as the runtime scale.
local function obj_natural_size(rel_path)
	local body = fs.readFile(target .. "/" .. rel_path)
	local minx, miny, minz = math.huge, math.huge, math.huge
	local maxx, maxy, maxz = -math.huge, -math.huge, -math.huge
	for x, y, z in body:gmatch("v ([%-%d%.eE+]+) ([%-%d%.eE+]+) ([%-%d%.eE+]+)") do
		x, y, z = tonumber(x), tonumber(y), tonumber(z)
		if x < minx then minx = x end
		if y < miny then miny = y end
		if z < minz then minz = z end
		if x > maxx then maxx = x end
		if y > maxy then maxy = y end
		if z > maxz then maxz = z end
	end
	local sx, sy, sz = maxx - minx, maxy - miny, maxz - minz
	if sx < 1e-6 then sx = 1 end
	if sy < 1e-6 then sy = 1 end
	if sz < 1e-6 then sz = 1 end
	return sx, sy, sz
end

local natural_size = {}
for id, rel in pairs(copied_meshes) do
	local sx, sy, sz = obj_natural_size(rel)
	natural_size[id] = { sx, sy, sz }
end

-- Shepperd's method: quaternion from a 3x3 rotation matrix. Numerically
-- stable across all rotations including the 180-degree edge case.
local function decompose_cframe(c)
	local x, y, z = c[1], c[2], c[3]
	local r00, r01, r02 = c[4], c[5], c[6]
	local r10, r11, r12 = c[7], c[8], c[9]
	local r20, r21, r22 = c[10], c[11], c[12]
	local trace = r00 + r11 + r22
	local qw, qx, qy, qz
	if trace > 0 then
		local s = math.sqrt(trace + 1) * 2
		qw = 0.25 * s
		qx = (r21 - r12) / s
		qy = (r02 - r20) / s
		qz = (r10 - r01) / s
	elseif r00 > r11 and r00 > r22 then
		local s = math.sqrt(1 + r00 - r11 - r22) * 2
		qw = (r21 - r12) / s
		qx = 0.25 * s
		qy = (r01 + r10) / s
		qz = (r02 + r20) / s
	elseif r11 > r22 then
		local s = math.sqrt(1 + r11 - r00 - r22) * 2
		qw = (r02 - r20) / s
		qx = (r01 + r10) / s
		qy = 0.25 * s
		qz = (r12 + r21) / s
	else
		local s = math.sqrt(1 + r22 - r00 - r11) * 2
		qw = (r10 - r01) / s
		qx = (r02 + r20) / s
		qy = (r12 + r21) / s
		qz = 0.25 * s
	end
	local clamped = math.max(-1, math.min(1, qw))
	local angle = 2 * math.acos(clamped)
	local sh = math.sqrt(math.max(0, 1 - clamped * clamped))
	local ax, ay, az = 1, 0, 0
	if sh >= 1e-6 then
		ax, ay, az = qx / sh, qy / sh, qz / sh
	end
	return x, y, z, angle, ax, ay, az
end

local cx, cy, cz = 0, 0, 0
local min_x, min_y, min_z = math.huge, math.huge, math.huge
local max_x, max_y, max_z = -math.huge, -math.huge, -math.huge
for _, mp in ipairs(parts) do
	local x, y, z = mp.cframe[1], mp.cframe[2], mp.cframe[3]
	cx, cy, cz = cx + x, cy + y, cz + z
	if x < min_x then min_x = x end
	if y < min_y then min_y = y end
	if z < min_z then min_z = z end
	if x > max_x then max_x = x end
	if y > max_y then max_y = y end
	if z > max_z then max_z = z end
end
if #parts > 0 then
	cx, cy, cz = cx / #parts, cy / #parts, cz / #parts
end
local span = math.max(max_x - min_x, max_y - min_y, max_z - min_z, 4)
local cam_dist = span * 1.5

local lines = {}
local function emit(s)
	table.insert(lines, s)
end
local function f(fmt, ...)
	emit(string.format(fmt, ...))
end

emit("-- Generated by src/scene.lua from a Roblox .rbxl export.")
emit("---@diagnostic disable: undefined-global, redundant-parameter, undefined-field, inject-field, assign-type-mismatch")
emit("")
emit("local camera = Camera.new()")
emit("camera.fov = 70")
emit("camera.near_clip = 0.1")
f("camera.far_clip = %d", math.max(1000, math.floor(cam_dist * 10)))
emit("")
emit("local meshes = {}")
emit("local textures = {}")
emit("")

for _, id in ipairs(util.sorted_keys(copied_meshes)) do
	f("meshes[%q] = Mesh.new(COMMON_PATH .. %q)", id, copied_meshes[id])
end
emit("")
for _, id in ipairs(util.sorted_keys(copied_textures)) do
	f("textures[%q] = Image.new(COMMON_PATH .. %q)", id, copied_textures[id])
end
emit("")
emit("local function make_part(mesh_id, texture_id, transform)")
emit("    local o = Object.new()")
emit("    o.transform = transform")
emit("    o.parent = Scene")
emit("    o:add_component(ECS_MESH_COMPONENT, { mesh = meshes[mesh_id] })")
emit("    if texture_id then o:add_component(ECS_MESH_TEXTURE_COMPONENT, { texture = textures[texture_id] }) end")
emit("    return o")
emit("end")
emit("")

local EPS = 1e-6
for _, mp in ipairs(parts) do
	local x, y, z, angle, ax, ay, az = decompose_cframe(mp.cframe)
	local nat = natural_size[mp.mesh_id]
	local sx, sy, sz = 1, 1, 1
	if mp.size and nat then
		sx, sy, sz = mp.size.x / nat[1], mp.size.y / nat[2], mp.size.z / nat[3]
	end
	local rot_part = (math.abs(angle) < EPS) and ""
		or string.format(":rotate(%g, Vec3.new(%g, %g, %g))", angle, ax, ay, az)
	local scale_part = (math.abs(sx - 1) < EPS and math.abs(sy - 1) < EPS and math.abs(sz - 1) < EPS) and ""
		or string.format(":scale(Vec3.new(%g, %g, %g))", sx, sy, sz)
	local transform = string.format(
		"Mat4.new(1):translate(Vec3.new(%g, %g, %g))%s%s",
		x, y, z, rot_part, scale_part
	)
	f("-- %s", mp.path)
	f("make_part(%q, %s, %s)", mp.mesh_id, mp.texture_id and string.format("%q", mp.texture_id) or "nil", transform)
end

local cam_min = math.max(0.5, span * 0.25)
local cam_max = math.max(5, span * 5)
emit("")
f("local CENTROID = Vec3.new(%g, %g, %g)", cx, cy, cz)
f("local CAM_DIST = %g", cam_dist)
f("local CAM_MIN, CAM_MAX = %g, %g", cam_min, cam_max)
emit("World.rendering.camera = camera")
emit("World.rendering.step:connect(function(delta_time)")
emit('    if ImGui.Begin("Scene") then')
emit('        _, CAM_DIST = ImGui.SliderFloat("Camera distance", CAM_DIST, CAM_MIN, CAM_MAX)')
emit("        ImGui.End()")
emit("    end")
emit("    local t = (now() / 5000) % (math.pi * 2)")
emit("    camera.transform = Mat4.new(1)")
emit("        :translate(CENTROID)")
emit("        :rotate(t, Vec3.new(0, 1, 0))")
emit("        :translate(Vec3.new(0, CAM_DIST * 0.3, CAM_DIST))")
emit("    Gizmo.set_line_width(0.05)")
emit("    Gizmo.draw_grid(100, 100, Vec3.new(0.25, 0.25, 0.25))")
emit("    World.rendering.render_pass()")
emit("end)")

fs.writeFile(target .. "/entry.lua", table.concat(lines, "\n") .. "\n")
fs.writeFile(target .. "/config.lua", table.concat({
	"BSGE = {",
	'	default_asset_directory = "./",',
	"}",
	"COMMON_PATH = BSGE.default_asset_directory",
	"",
}, "\n"))

print(string.format("wrote %s/entry.lua  (%d lines)", target, #lines))
print(string.format("wrote %s/config.lua", target))

-- If the target landed inside <luabsge_root>/projects/<name>/, emit a run hint.
local project_name = target:match("^%.%./%.%./(.+)$")
if project_name then
	print("")
	print("next: from the luabsge root,  bash run.sh " .. project_name .. "  (or run_win.bat " .. project_name .. ")")
end
