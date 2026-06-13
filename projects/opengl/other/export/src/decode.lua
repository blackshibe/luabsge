-- Phase 5 + 6: decode raw downloads and finalize the manifest.
-- Meshes go through the filemesh module -> .obj. Animations are rbxm-encoded
-- KeyframeSequences; we walk them to JSON keeping the Pose hierarchy intact
-- (Roblox stores Pose CFrames relative to the parent — flattening loses them).
--
--   lune run src/decode.lua [<out_dir>]

local fs = require("@lune/fs")
local serde = require("@lune/serde")
local roblox = require("@lune/roblox")
local process = require("@lune/process")
local filemesh = require("./filemesh")
local util = require("./common/util")
local manifest_lib = require("./common/manifest")

local args = process.args
local out_root = args[1] or "out"

local manifest, err = manifest_lib.read(out_root)
if not manifest then
	print("error: " .. err)
	process.exit(1)
end
local assets = manifest.assets or {}
local ids = util.sorted_keys(assets)

local stats = { mesh_ok = 0, mesh_fail = 0, anim_ok = 0, anim_fail = 0 }

local function decode_mesh(id, entry)
	if not entry.file or not entry.file:find("%.mesh$") then
		return "skip"
	end
	local raw = out_root .. "/" .. entry.file
	if not fs.isFile(raw) then
		return "skip"
	end

	local obj_rel = entry.file:gsub("%.mesh$", ".obj")
	local obj_abs = out_root .. "/" .. obj_rel
	if fs.isFile(obj_abs) then
		entry.raw = entry.file
		entry.file = obj_rel
		return "skip"
	end

	local ok, info = filemesh.convert(raw, obj_abs)
	if not ok then
		entry.decode_status = "filemesh_failed: " .. tostring(info)
		return "fail"
	end
	entry.raw = entry.file
	entry.file = obj_rel
	entry.decode_status = "ok"
	entry.mesh_version = string.format("%d.%02d", info.major, info.minor)
	entry.vertex_count = #info.verts
	entry.face_count = #info.faces
	return "ok"
end

local function pose_to_table(pose)
	local children
	for _, c in ipairs(pose:GetChildren()) do
		if c.ClassName == "Pose" then
			children = children or {}
			children[c.Name] = pose_to_table(c)
		end
	end
	return {
		cframe = { pose.CFrame:GetComponents() },
		easing_style = tostring(pose.EasingStyle):gsub("^Enum%.[^.]+%.", ""),
		easing_direction = tostring(pose.EasingDirection):gsub("^Enum%.[^.]+%.", ""),
		weight = pose.Weight,
		children = children,
	}
end

local function keyframe_sequence_to_table(seq)
	local frames = {}
	for _, kf in ipairs(seq:GetChildren()) do
		if kf.ClassName == "Keyframe" then
			local poses = {}
			for _, c in ipairs(kf:GetChildren()) do
				if c.ClassName == "Pose" then
					poses[c.Name] = pose_to_table(c)
				end
			end
			table.insert(frames, {
				time = kf.Time,
				name = kf.Name ~= "" and kf.Name or nil,
				poses = poses,
			})
		end
	end
	table.sort(frames, function(a, b)
		return a.time < b.time
	end)
	return {
		name = seq.Name,
		loop = seq.Loop,
		priority = tostring(seq.Priority):gsub("^Enum%.AnimationPriority%.", ""),
		authored_hip_height = seq.AuthoredHipHeight,
		keyframes = frames,
	}
end

local function decode_animation(id, entry)
	local is_rbxm = entry.file and entry.file:find("%.rbxm$")
	if not (entry.kind == "animation" or is_rbxm) then
		return "skip"
	end
	if not entry.file then
		return "skip"
	end

	local raw_abs = out_root .. "/" .. entry.file
	if not fs.isFile(raw_abs) then
		return "skip"
	end

	local out_rel = entry.file:gsub("%.rbxm$", ".keyframes.json"):gsub("^model/", "animation/")
	local out_abs = out_root .. "/" .. out_rel
	if fs.isFile(out_abs) then
		entry.raw = entry.file
		entry.file = out_rel
		return "skip"
	end

	local ok, roots = pcall(roblox.deserializeModel, fs.readFile(raw_abs))
	if not ok then
		entry.decode_status = "deserialize_failed"
		return "fail"
	end

	local seq
	for _, root in ipairs(roots) do
		if root.ClassName == "KeyframeSequence" then
			seq = root
			break
		end
	end
	if not seq then
		entry.decode_status = "not_a_keyframe_sequence"
		return "skip"
	end

	local decoded = keyframe_sequence_to_table(seq)
	local out_dir = out_abs:match("(.*)/[^/]+$")
	if out_dir and not fs.isDir(out_dir) then
		fs.writeDir(out_dir)
	end
	fs.writeFile(out_abs, serde.encode("json", decoded, true))

	entry.raw = entry.file
	entry.file = out_rel
	entry.kind = "animation"
	entry.decode_status = "ok"
	return "ok"
end

print(string.format("decoding %d assets in %s", #ids, out_root))

for _, id in ipairs(ids) do
	local entry = assets[id]

	local r_mesh = decode_mesh(id, entry)
	if r_mesh ~= "skip" then
		stats["mesh_" .. r_mesh] = stats["mesh_" .. r_mesh] + 1
		print(string.format("  mesh  %-12s -> %s  %s", id, r_mesh, entry.file or ""))
	end

	local r_anim = decode_animation(id, entry)
	if r_anim ~= "skip" then
		stats["anim_" .. r_anim] = stats["anim_" .. r_anim] + 1
		print(string.format("  anim  %-12s -> %s  %s", id, r_anim, entry.file or ""))
	end
end

manifest_lib.write(out_root, manifest)

print(string.format(
	"\ndone. meshes: ok=%d fail=%d   animations: ok=%d fail=%d",
	stats.mesh_ok, stats.mesh_fail, stats.anim_ok, stats.anim_fail
))
