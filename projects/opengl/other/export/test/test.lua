-- End-to-end test runner. Regenerates both fixtures, runs the relevant
-- pipeline stage against each, and asserts on the produced output.
-- Replaces the previous bash+grep+python suite — everything that wants to
-- inspect JSON or read a file goes through @lune/serde and @lune/fs directly.
--
-- Usage:
--   lune run test.lua
-- Exits with the number of failing assertions (0 == green).

local fs = require("@lune/fs")
local serde = require("@lune/serde")
local process = require("@lune/process")

-- ---------------------------------------------------------------------------
-- Assertion harness
-- ---------------------------------------------------------------------------

local pass, fail = 0, 0

local function ok(desc)
	pass = pass + 1
	print("  ok   " .. desc)
end

local function bad(desc, detail)
	fail = fail + 1
	print("  FAIL " .. desc .. (detail and ("  -- " .. detail) or ""))
end

local function assert_true(desc, cond, detail)
	if cond then
		ok(desc)
	else
		bad(desc, detail)
	end
end

local function assert_file(desc, path)
	assert_true(desc, fs.isFile(path), "missing: " .. path)
end

local function assert_contains(desc, path, needle)
	if not fs.isFile(path) then
		bad(desc, "missing: " .. path)
		return
	end
	local body = fs.readFile(path)
	-- plain=true so we don't have to escape JSON punctuation
	assert_true(desc, body:find(needle, 1, true) ~= nil, "not found: " .. needle)
end

local function read_json(path)
	return serde.decode("json", fs.readFile(path))
end

local function run_lune(script, ...)
	local r = process.exec("lune", { "run", script, ... }, { stdio = "default" })
	assert_true("ran " .. script, r.ok, "exit code " .. tostring(r.code))
	return r
end

local function rm(path)
	if fs.isDir(path) then
		fs.removeDir(path)
	elseif fs.isFile(path) then
		fs.removeFile(path)
	end
end

-- ---------------------------------------------------------------------------
-- Phase 1-3: export against the script-heavy fixture rbxl.
-- ---------------------------------------------------------------------------

local FIXTURE = "test/fixture.rbxl"
local OUT = "test/out"

print("==> regenerating phase 1-3 fixture")
rm(OUT)
rm(FIXTURE)
run_lune("test/gen_fixture.lua")

print("==> running exporter against fixture")
run_lune("src/export.lua", FIXTURE, OUT)

print("==> asserting script files on disk")
assert_file("ModuleScript with children -> init.luau", OUT .. "/scripts/ReplicatedStorage/Lib/init.luau")
assert_file("leaf ModuleScript -> .luau", OUT .. "/scripts/ReplicatedStorage/Lib/Helper.luau")
assert_file("Script with children -> init.server.luau", OUT .. "/scripts/ServerScriptService/RootServer/init.server.luau")
assert_file("leaf LocalScript -> .client.luau", OUT .. "/scripts/ServerScriptService/RootServer/Inner.client.luau")

print("==> asserting source contents")
assert_contains(
	"RootServer source preserved",
	OUT .. "/scripts/ServerScriptService/RootServer/init.server.luau",
	"hello from server"
)
assert_contains("Lib source preserved", OUT .. "/scripts/ReplicatedStorage/Lib/init.luau", "Lib.icon")

print("==> asserting datamodel + manifest")
assert_file("datamodel.json written", OUT .. "/datamodel.json")
assert_file("manifest.json written", OUT .. "/manifest.json")

print("==> asserting script-source asset scan")
local manifest = read_json(OUT .. "/manifest.json")
for _, id in ipairs({ "6022668898", "6764432408", "7059346373", "9120389081" }) do
	local entry = manifest.assets and manifest.assets[id]
	assert_true("manifest contains asset " .. id, entry ~= nil)
	if entry then
		assert_true(
			"  asset " .. id .. " attributed to a script",
			entry.refs and entry.refs[1] and entry.refs[1].script ~= nil
		)
	end
end

print("==> asserting source_file pointers match disk")
local datamodel = read_json(OUT .. "/datamodel.json")
local function find_source_files(node, out)
	out = out or {}
	if node.source_file then
		out[node.source_file] = true
	end
	for _, child in pairs(node.children or {}) do
		find_source_files(child, out)
	end
	return out
end
local pointers = find_source_files(datamodel)
for _, p in ipairs({
	"scripts/ReplicatedStorage/Lib/init.luau",
	"scripts/ReplicatedStorage/Lib/Helper.luau",
	"scripts/ServerScriptService/RootServer/init.server.luau",
	"scripts/ServerScriptService/RootServer/Inner.client.luau",
}) do
	assert_true("datamodel.source_file = " .. p, pointers[p] == true)
	assert_file("  and disk has " .. p, OUT .. "/" .. p)
end

-- ---------------------------------------------------------------------------
-- Phase 5 + 6: decoders + manifest finalize.
-- ---------------------------------------------------------------------------

local DECODE_IN = "test/decode_in"

print("")
print("==> regenerating decode fixture")
rm(DECODE_IN)
run_lune("test/gen_decode_fixture.lua")

print("==> running decoder against fixture")
run_lune("src/decode.lua", DECODE_IN)

print("==> asserting decoded files on disk")
assert_file("mesh -> .obj", DECODE_IN .. "/mesh/100.obj")
assert_file("animation -> .keyframes.json", DECODE_IN .. "/animation/200.keyframes.json")
assert_file("raw mesh kept", DECODE_IN .. "/mesh/100.mesh")
assert_file("raw rbxm kept", DECODE_IN .. "/animation/200.rbxm")

print("==> asserting OBJ content")
local obj = fs.readFile(DECODE_IN .. "/mesh/100.obj")
local function count_lines_matching(text, pattern)
	local n = 0
	for _ in text:gmatch(pattern) do
		n = n + 1
	end
	return n
end
assert_true("OBJ has 3 verts", count_lines_matching(obj, "\nv [^\n]+") == 3)
assert_true("OBJ has 3 normals", count_lines_matching(obj, "\nvn [^\n]+") == 3)
assert_true("OBJ has 3 uvs", count_lines_matching(obj, "\nvt [^\n]+") == 3)
assert_true("OBJ has 1 face", count_lines_matching(obj, "\nf [^\n]+") == 1)

print("==> asserting keyframes content")
local anim = read_json(DECODE_IN .. "/animation/200.keyframes.json")
assert_true("anim name preserved", anim.name == "TestAnim")
assert_true("anim loop preserved", anim.loop == true)
assert_true("anim has 2 keyframes", #anim.keyframes == 2)
assert_true("Frame1 present", anim.keyframes[1].name == "Frame1")
assert_true("Frame2 present", anim.keyframes[2].name == "Frame2")
assert_true(
	"pose tree (Torso under HumanoidRootPart)",
	anim.keyframes[1].poses
		and anim.keyframes[1].poses.HumanoidRootPart
		and anim.keyframes[1].poses.HumanoidRootPart.children
		and anim.keyframes[1].poses.HumanoidRootPart.children.Torso ~= nil
)
assert_true(
	"CFrame components captured (12 floats)",
	type(anim.keyframes[1].poses.HumanoidRootPart.cframe) == "table"
		and #anim.keyframes[1].poses.HumanoidRootPart.cframe == 12
)

print("==> asserting manifest finalize")
local decode_manifest = read_json(DECODE_IN .. "/manifest.json")
local mesh_entry = decode_manifest.assets["100"]
local anim_entry = decode_manifest.assets["200"]
assert_true("mesh entry has file=.obj", mesh_entry.file == "mesh/100.obj")
assert_true("mesh entry has raw=.mesh", mesh_entry.raw == "mesh/100.mesh")
assert_true("mesh entry records vertex_count", mesh_entry.vertex_count == 3)
assert_true("mesh entry records face_count", mesh_entry.face_count == 1)
assert_true("anim entry has file=.json", anim_entry.file == "animation/200.keyframes.json")
assert_true("anim entry has raw=.rbxm", anim_entry.raw == "animation/200.rbxm")

-- ---------------------------------------------------------------------------
-- Summary
-- ---------------------------------------------------------------------------

print("")
print(string.format("==> result: %d passed, %d failed", pass, fail))
process.exit(fail > 0 and 1 or 0)
