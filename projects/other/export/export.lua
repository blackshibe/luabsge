-- Exports a .rbxl place file to:
--   <out>/datamodel.json      full instance tree, every readable serialized property
--   <out>/manifest.json       asset_id -> { kind, refs[] }; downloader reads this
--   <out>/scripts/<path>.luau every Script / LocalScript / ModuleScript .Source
--
-- Usage:
--   lune run export.lua [<input.rbxl>] [<out_dir>]
--
-- Properties are enumerated via @lune/roblox's reflection database
-- (`roblox.getReflectionDatabase()`); no per-class allowlist. Each value is
-- typed via `typeof()` and serialized to a JSON-friendly form that keeps the
-- type information (`__type = "CFrame"`, etc.) so the importer can rebuild
-- it without guessing.

local fs = require("@lune/fs")
local process = require("@lune/process")
local serde = require("@lune/serde")
local roblox = require("@lune/roblox")

local args = process.args
local in_path = args[1] or "in/import.rbxl"
local out_root = args[2] or "out"

-- ---------------------------------------------------------------------------
-- Reflection
-- ---------------------------------------------------------------------------

local db = roblox.getReflectionDatabase()

-- Walks Superclass and returns a flat map of every property descriptor visible
-- on `class_name`. Cached because every instance of the same class hits this.
local class_props_cache = {}
local function class_props(class_name)
	local cached = class_props_cache[class_name]
	if cached then
		return cached
	end
	local out = {}
	local cls = db:GetClass(class_name)
	while cls do
		for name, prop in pairs(cls.Properties) do
			if out[name] == nil then
				out[name] = prop
			end
		end
		cls = cls.Superclass and db:GetClass(cls.Superclass) or nil
	end
	class_props_cache[class_name] = out
	return out
end

-- Properties we never want in the JSON, regardless of class:
--   Source      handled separately (written to disk under scripts/)
--   Parent      redundant; the tree structure already encodes parentage
--   UniqueId    randomly generated; serialization adds noise without value
local SKIP_PROPS = {
	Source = true,
	Parent = true,
	UniqueId = true,
	HistoryId = true,
	ScriptGuid = true,
}

-- Tags that mean "don't bother dumping". rbx-dom marks computed,
-- not-actually-stored, or editor-only properties with these.
local function tag_skip(prop)
	for _, t in ipairs(prop.Tags) do
		if t == "Hidden" or t == "Deprecated" or t == "NotScriptable" then
			return true
		end
	end
	return false
end

-- ---------------------------------------------------------------------------
-- Asset collection
-- ---------------------------------------------------------------------------

-- Property name → kind. Hits via either a plain string property containing an
-- rbxassetid:// URI, or a Content userdata's .Uri. Unknown names default to
-- "unknown" and get sniffed by magic bytes later, in the downloader phase.
local ASSET_KIND_BY_PROP = {
	-- Meshes
	MeshContent = "mesh",
	MeshId = "mesh",
	-- Images / textures
	TextureContent = "image",
	TextureID = "image",
	TextureId = "image",
	Texture = "image",
	Image = "image",
	ImageContent = "image",
	ColorMap = "image",
	NormalMap = "image",
	MetalnessMap = "image",
	RoughnessMap = "image",
	SkyboxBk = "image",
	SkyboxDn = "image",
	SkyboxFt = "image",
	SkyboxLf = "image",
	SkyboxRt = "image",
	SkyboxUp = "image",
	SunTextureId = "image",
	MoonTextureId = "image",
	ShirtTemplate = "image",
	PantsTemplate = "image",
	Graphic = "image",
	-- Sounds
	SoundId = "sound",
	AudioContent = "sound",
	-- Animations
	AnimationId = "animation",
	-- Video
	Video = "video",
	VideoContent = "video",
}

local assets = {}

local function register_asset(uri, instance_path, prop_name)
	local id = uri:match("^rbxassetid://(%d+)$")
	if not id then
		return
	end
	local kind = (prop_name and ASSET_KIND_BY_PROP[prop_name]) or "unknown"
	local entry = assets[id]
	if not entry then
		entry = { kind = kind, refs = {} }
		assets[id] = entry
	elseif entry.kind == "unknown" and kind ~= "unknown" then
		entry.kind = kind
	end
	table.insert(entry.refs, {
		instance = instance_path,
		property = prop_name,
	})
end

-- ---------------------------------------------------------------------------
-- Value serialization
-- ---------------------------------------------------------------------------

local function serialize_value(value, prop_name, instance_path)
	local t = typeof(value)

	if t == "string" then
		if value:find("^rbxassetid://") then
			register_asset(value, instance_path, prop_name)
		end
		return value
	elseif t == "number" or t == "boolean" then
		return value
	elseif t == "nil" then
		return nil
	elseif t == "Content" then
		local uri = value.Uri
		if uri then
			register_asset(uri, instance_path, prop_name)
		end
		return { __type = "Content", uri = uri, source_type = tostring(value.SourceType) }
	elseif t == "CFrame" then
		return { __type = "CFrame", components = { value:GetComponents() } }
	elseif t == "Vector3" then
		return { __type = "Vector3", x = value.X, y = value.Y, z = value.Z }
	elseif t == "Vector2" then
		return { __type = "Vector2", x = value.X, y = value.Y }
	elseif t == "Color3" then
		return { __type = "Color3", r = value.R, g = value.G, b = value.B }
	elseif t == "EnumItem" then
		return {
			__type = "EnumItem",
			enum = tostring(value.EnumType):gsub("^Enum%.", ""),
			name = value.Name,
			value = value.Value,
		}
	elseif t == "UDim" then
		return { __type = "UDim", scale = value.Scale, offset = value.Offset }
	elseif t == "UDim2" then
		return {
			__type = "UDim2",
			x = { scale = value.X.Scale, offset = value.X.Offset },
			y = { scale = value.Y.Scale, offset = value.Y.Offset },
		}
	elseif t == "BrickColor" then
		return { __type = "BrickColor", name = value.Name, number = value.Number }
	elseif t == "Rect" then
		return {
			__type = "Rect",
			min = { x = value.Min.X, y = value.Min.Y },
			max = { x = value.Max.X, y = value.Max.Y },
		}
	elseif t == "Font" then
		return {
			__type = "Font",
			family = value.Family,
			weight = tostring(value.Weight),
			style = tostring(value.Style),
		}
	elseif t == "NumberRange" then
		return { __type = "NumberRange", min = value.Min, max = value.Max }
	elseif t == "NumberSequence" then
		local kps = {}
		for i, kp in ipairs(value.Keypoints) do
			kps[i] = { time = kp.Time, value = kp.Value, envelope = kp.Envelope }
		end
		return { __type = "NumberSequence", keypoints = kps }
	elseif t == "ColorSequence" then
		local kps = {}
		for i, kp in ipairs(value.Keypoints) do
			kps[i] = {
				time = kp.Time,
				value = { r = kp.Value.R, g = kp.Value.G, b = kp.Value.B },
			}
		end
		return { __type = "ColorSequence", keypoints = kps }
	elseif t == "PhysicalProperties" then
		return {
			__type = "PhysicalProperties",
			density = value.Density,
			friction = value.Friction,
			elasticity = value.Elasticity,
			friction_weight = value.FrictionWeight,
			elasticity_weight = value.ElasticityWeight,
		}
	elseif t == "Vector3int16" then
		return { __type = "Vector3int16", x = value.X, y = value.Y, z = value.Z }
	elseif t == "Vector2int16" then
		return { __type = "Vector2int16", x = value.X, y = value.Y }
	elseif t == "Ray" then
		return {
			__type = "Ray",
			origin = { x = value.Origin.X, y = value.Origin.Y, z = value.Origin.Z },
			direction = { x = value.Direction.X, y = value.Direction.Y, z = value.Direction.Z },
		}
	elseif t == "Instance" then
		-- Refs to other instances aren't useful in JSON without IDs we control;
		-- drop them. (Importer rebuilds these from datamodel structure if needed.)
		return nil
	end

	-- Anything else: best-effort string form so we at least know what we lost.
	return { __type = "unknown:" .. t, repr = tostring(value) }
end

-- ---------------------------------------------------------------------------
-- Filesystem helpers
-- ---------------------------------------------------------------------------

-- Replaces path-unsafe characters; collapses whitespace. Returns "" → caller
-- substitutes ClassName.
local function sanitize(name)
	local s = name:gsub('[%c/\\:*?"<>|]+', "_"):gsub("^%s+", ""):gsub("%s+$", "")
	return s
end

local scripts_written = {}

local function write_script(path, source)
	local target = out_root .. "/scripts/" .. path .. ".luau"
	local dir = target:match("(.*)/[^/]+$")
	if dir and not fs.isDir(dir) then
		fs.writeDir(dir)
	end
	fs.writeFile(target, source)
	scripts_written[path] = source
end

-- ---------------------------------------------------------------------------
-- Walk
-- ---------------------------------------------------------------------------

local function dump_instance(inst, path)
	local class_name = inst.ClassName
	local node = {
		class_name = class_name,
		name = inst.Name,
		properties = {},
		children = {},
	}

	-- Script source: write to disk, leave a pointer in the JSON.
	if class_name == "Script" or class_name == "LocalScript" or class_name == "ModuleScript" then
		local ok, src = pcall(function()
			return inst.Source
		end)
		if ok and type(src) == "string" and #src > 0 then
			write_script(path, src)
			node.source_file = "scripts/" .. path .. ".luau"
		end
	end

	-- Properties via reflection.
	local props = class_props(class_name)
	for name, prop in pairs(props) do
		if not SKIP_PROPS[name] and not tag_skip(prop) then
			local ok, val = pcall(function()
				return inst[name]
			end)
			if ok and val ~= nil then
				local encoded = serialize_value(val, name, path)
				if encoded ~= nil then
					node.properties[name] = encoded
				end
			end
		end
	end

	-- Children, with collision-safe keys so duplicate sibling names don't
	-- overwrite each other in the JSON object.
	local seen = {}
	for _, child in ipairs(inst:GetChildren()) do
		local raw = sanitize(child.Name)
		if raw == "" then
			raw = child.ClassName
		end
		local key = raw
		local n = 1
		while seen[key] do
			n = n + 1
			key = raw .. "_" .. n
		end
		seen[key] = true
		node.children[key] = dump_instance(child, path .. "/" .. key)
	end

	return node
end

-- ---------------------------------------------------------------------------
-- Main
-- ---------------------------------------------------------------------------

local function count(tbl)
	local n = 0
	for _ in pairs(tbl) do
		n = n + 1
	end
	return n
end

print("loading: " .. in_path)
local bytes = fs.readFile(in_path)
print("  " .. #bytes .. " bytes")

local game = roblox.deserializePlace(bytes)

if not fs.isDir(out_root) then
	fs.writeDir(out_root)
end

local datamodel = { class_name = "DataModel", children = {} }
for _, svc in ipairs(game:GetChildren()) do
	local raw = sanitize(svc.Name)
	if raw == "" then
		raw = svc.ClassName
	end
	local key = raw
	local n = 1
	while datamodel.children[key] do
		n = n + 1
		key = raw .. "_" .. n
	end
	print("  service: " .. key)
	datamodel.children[key] = dump_instance(svc, key)
end

-- Script-source scan: pick up any rbxassetid:// literals that only appear
-- inside script code, not on a property.
for path, src in pairs(scripts_written) do
	for id in src:gmatch("rbxassetid://(%d+)") do
		local entry = assets[id]
		if not entry then
			entry = { kind = "unknown", refs = {} }
			assets[id] = entry
		end
		table.insert(entry.refs, { script = path })
	end
end

fs.writeFile(out_root .. "/datamodel.json", serde.encode("json", datamodel, true))
fs.writeFile(out_root .. "/manifest.json", serde.encode("json", { assets = assets }, true))

print(string.format("done. assets=%d  scripts=%d  out=%s", count(assets), count(scripts_written), out_root))
