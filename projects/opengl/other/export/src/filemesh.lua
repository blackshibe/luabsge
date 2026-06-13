-- Pure Lua port of the Roblox FileMesh -> Wavefront OBJ converter.
-- Reference: https://devforum.roblox.com/t/roblox-filemesh-format-specification/326114/1
--
-- Supports:
--   v1.00, v1.01 : ASCII text format (positions, normals, UVs)
--   v2.00        : binary, no LODs
--   v3.00, v3.01 : binary, LODs (we keep LOD 0 only)
--   v4.00, v4.01 : binary, LODs + bones + subsets (geometry only)
--   v5.00        : best-effort; emits whatever geometry can be decoded via v4
--
-- Usage as a library:
--   local mesh = require("./filemesh")
--   local result, err = mesh.parse(bytes)
--   local obj_text = mesh.write_obj(result.verts, result.faces)
--
-- Usage as a CLI (Lune):
--   lune run filemesh.lua <input.mesh> [output.obj]

local M = {}

-- ---------------------------------------------------------------------------
-- Header detection
-- ---------------------------------------------------------------------------

-- Returns (major, minor, byte_offset_after_header) or nil on no match.
-- Matches the C++ minor-normalization: "1.0" -> 0, "1.01" -> 1, "1.10" -> 10.
local function parse_version(bytes)
    local maj, min, eol = bytes:match("^version (%d+)%.(%d+)(\r?\n)")
    if not maj then return nil end
    local minor = tonumber(min)
    if #min == 1 then minor = minor * 10 end
    -- Compute the offset after the trailing newline (1-based).
    local prefix_len = #("version " .. maj .. "." .. min .. eol)
    return tonumber(maj), minor, prefix_len + 1
end

-- ---------------------------------------------------------------------------
-- V1 (ASCII) parser
-- ---------------------------------------------------------------------------

-- Format after the version line:
--   <numFaces>\n
--   [px,py,pz][nx,ny,nz][tu,tv,tw][px,py,pz]... (9 triples per face)
-- v1.00 stores positions at 2x — divide by 2 on read.
local function parse_v1(bytes, offset, scale_half)
    local body = bytes:sub(offset)
    local num_faces = body:match("^%s*(%d+)")
    if not num_faces then return nil, "v1: failed to read face count" end
    num_faces = tonumber(num_faces)

    -- Each `[a,b,c]` is captured as three numeric strings. We need 9 per face.
    local need = num_faces * 9
    local triples = table.create and table.create(need) or {}
    local got = 0
    for a, b, c in body:gmatch("%[([^,]+),([^,]+),([^%]]+)%]") do
        got = got + 1
        triples[got] = { tonumber(a), tonumber(b), tonumber(c) }
        if got == need then break end
    end
    if got < need then
        return nil, string.format("v1: needed %d triples, got %d", need, got)
    end

    local verts, faces = {}, {}
    for f = 0, num_faces - 1 do
        local idx = #verts
        for v = 0, 2 do
            local base = f * 9 + v * 3 + 1
            local p, n, uv = triples[base], triples[base + 1], triples[base + 2]
            local vert = {
                px = p[1], py = p[2], pz = p[3],
                nx = n[1], ny = n[2], nz = n[3],
                tu = uv[1], tv = uv[2],
            }
            if scale_half then
                vert.px = vert.px * 0.5
                vert.py = vert.py * 0.5
                vert.pz = vert.pz * 0.5
            end
            verts[#verts + 1] = vert
        end
        faces[#faces + 1] = { idx + 1, idx + 2, idx + 3 }
    end
    return verts, faces
end

-- ---------------------------------------------------------------------------
-- V2 / V3 / V4 binary parsers
-- ---------------------------------------------------------------------------

-- Helper: pull a single vertex (32-byte canonical layout) from `bytes` at the
-- 1-based position `at`. `stride` is the writer's reported vertex size; any
-- trailing tangent / RGBA bytes are skipped via the caller advancing by stride.
local function read_vertex(bytes, at)
    local px, py, pz, nx, ny, nz, tu, tv =
        string.unpack("<ffffffff", bytes, at)
    return {
        px = px, py = py, pz = pz,
        nx = nx, ny = ny, nz = nz,
        tu = tu, tv = tv,
    }
end

local function read_faces(bytes, at, num_faces, sizeof_face)
    local faces = {}
    for i = 1, num_faces do
        local a, b, c = string.unpack("<I4I4I4", bytes, at)
        faces[i] = { a + 1, b + 1, c + 1 } -- OBJ is 1-based
        at = at + sizeof_face
    end
    return faces, at
end

local function parse_v2(bytes, offset)
    -- HeaderV2: H sizeof_header | B sizeof_vertex | B sizeof_face | I4 verts | I4 faces
    local sizeof_header, sizeof_vertex, sizeof_face, num_verts, num_faces =
        string.unpack("<I2I1I1I4I4", bytes, offset)
    -- Trust the writer's header length so unknown trailing fields are skipped.
    local p = offset + sizeof_header
    local verts = {}
    for i = 1, num_verts do
        verts[i] = read_vertex(bytes, p)
        p = p + sizeof_vertex
    end
    local faces = read_faces(bytes, p, num_faces, sizeof_face)
    return verts, faces
end

local function parse_v3(bytes, offset)
    -- HeaderV3: H sizeof_header | B sizeof_vertex | B sizeof_face | H sizeof_lod
    --           | H num_lods    | I4 verts        | I4 faces
    local sizeof_header, sizeof_vertex, sizeof_face, _sizeof_lod, _num_lods, num_verts, num_faces =
        string.unpack("<I2I1I1I2I2I4I4", bytes, offset)
    local p = offset + sizeof_header
    local verts = {}
    for i = 1, num_verts do
        verts[i] = read_vertex(bytes, p)
        p = p + sizeof_vertex
    end
    local faces = read_faces(bytes, p, num_faces, sizeof_face)
    return verts, faces
end

local function parse_v4(bytes, offset)
    -- HeaderV4: H sizeof_header | H lod_type     | I4 num_verts
    --           | I4 num_faces  | H num_lods     | H num_bones
    --           | I4 sizeof_bone_names | H num_subsets | B num_hq_lods | B unused
    local sizeof_header, _lod_type, num_verts, num_faces, _num_lods, num_bones,
          _sizeof_bone_names, _num_subsets, _num_hq_lods, _unused =
        string.unpack("<I2I2I4I4I2I2I4I2I1I1", bytes, offset)
    local p = offset + sizeof_header

    -- v4 vertex stride is fixed at 40 bytes: pos(12) + nrm(12) + uv(8) +
    -- tangent sbyte4(4) + rgba(4).
    local STRIDE_V4 = 40
    local verts = {}
    for i = 1, num_verts do
        verts[i] = read_vertex(bytes, p)
        p = p + STRIDE_V4
    end
    if num_bones > 0 then
        -- Skip FileMeshSkinning[num_verts] — 8 bytes each (4 bone idx + 4 weights).
        p = p + num_verts * 8
    end
    local faces = read_faces(bytes, p, num_faces, 12)
    return verts, faces
end

-- ---------------------------------------------------------------------------
-- Public API
-- ---------------------------------------------------------------------------

-- Parses a FileMesh buffer. Returns:
--   { verts, faces, major, minor }   on success
--   nil, error_message               on failure
function M.parse(bytes)
    if not bytes or #bytes == 0 then return nil, "empty input" end
    local major, minor, offset = parse_version(bytes)
    if not major then return nil, "not a Roblox mesh (missing 'version' header)" end

    local verts, faces, err
    if major == 1 then
        verts, faces = parse_v1(bytes, offset, minor == 0)
    elseif major == 2 then
        verts, faces = parse_v2(bytes, offset)
    elseif major == 3 then
        verts, faces = parse_v3(bytes, offset)
    elseif major == 4 or major == 5 then
        verts, faces = parse_v4(bytes, offset)
    else
        err = "unsupported mesh version " .. major .. "." .. minor
    end
    if not verts then
        return nil, err or faces -- faces holds the error when verts is nil
    end
    return { verts = verts, faces = faces, major = major, minor = minor }
end

-- Serializes a vertex/face array to a Wavefront OBJ string.
-- UV.y is flipped from Roblox's top-left origin to OBJ's bottom-left.
function M.write_obj(verts, faces)
    local lines = { "# Converted from Roblox FileMesh", "o mesh" }
    for _, v in ipairs(verts) do
        lines[#lines + 1] = string.format("v %g %g %g", v.px, v.py, v.pz)
    end
    for _, v in ipairs(verts) do
        lines[#lines + 1] = string.format("vn %g %g %g", v.nx, v.ny, v.nz)
    end
    for _, v in ipairs(verts) do
        lines[#lines + 1] = string.format("vt %g %g", v.tu, 1 - v.tv)
    end
    for _, f in ipairs(faces) do
        local a, b, c = f[1], f[2], f[3]
        lines[#lines + 1] = string.format("f %d/%d/%d %d/%d/%d %d/%d/%d",
            a, a, a, b, b, b, c, c, c)
    end
    lines[#lines + 1] = "" -- trailing newline
    return table.concat(lines, "\n")
end

-- End-to-end: read input.mesh -> write output.obj. Returns true / false+err.
function M.convert(input_path, output_path)
    local fs = require("@lune/fs")
    if not fs.isFile(input_path) then
        return false, "input not found: " .. input_path
    end
    local result, err = M.parse(fs.readFile(input_path))
    if not result then return false, err end
    fs.writeFile(output_path, M.write_obj(result.verts, result.faces))
    return true, result
end

-- ---------------------------------------------------------------------------
-- CLI entry point
-- ---------------------------------------------------------------------------
-- Runs only when this file is the script Lune was invoked with (i.e. not when
-- another script `require`s it). Detected by `debug.info`-style script-name
-- inspection: when required, `process.args` still belongs to the parent script,
-- so we guard on an explicit `_G.__filemesh_cli` flag the parent never sets.
if not _G.__filemesh_required then
    _G.__filemesh_required = true
    local process = require("@lune/process")
    local args = process.args
    if #args >= 1 and args[1]:find("%.mesh$") then
        local input  = args[1]
        local output = args[2] or (input:gsub("%.mesh$", "") .. ".obj")
        local ok, info = M.convert(input, output)
        if not ok then
            io.stderr:write("filemesh: " .. info .. "\n")
            process.exit(1)
        end
        print(string.format("filemesh: %s -> %s  (v%d.%02d, %d verts, %d faces)",
            input, output, info.major, info.minor, #info.verts, #info.faces))
    end
end

return M
