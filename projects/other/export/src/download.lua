-- Downloads every asset listed in <out>/manifest.json from Roblox's CDN and
-- writes them to <out>/<kind>/<id>.<ext>. Resumable, concurrent.
--
--   lune run src/download.lua [<out_dir>] [--cookie <token> | --cookie-file <path>]
--
-- Auth precedence: --cookie  >  --cookie-file  >  $ROBLOSECURITY  >  .cookie

local fs = require("@lune/fs")
local net = require("@lune/net")
local task = require("@lune/task")
local process = require("@lune/process")
local util = require("./common/util")
local manifest_lib = require("./common/manifest")

local function parse_args(argv)
	local cli_cookie, cli_cookie_file
	local positionals = {}
	local i = 1
	while i <= #argv do
		local a = argv[i]
		if a == "--cookie" then
			cli_cookie = argv[i + 1]
			i = i + 2
		elseif a == "--cookie-file" then
			cli_cookie_file = argv[i + 1]
			i = i + 2
		else
			table.insert(positionals, a)
			i = i + 1
		end
	end
	return positionals, cli_cookie, cli_cookie_file
end

local positionals, CLI_COOKIE, CLI_COOKIE_FILE = parse_args(process.args)
local out_root = positionals[1] or "out"

local CONCURRENCY = 8
local MAX_ATTEMPTS = 3

-- Returns (kind, extension). "model" is the catch-all rbxm bucket; animations
-- land there when not pre-tagged via property reflection.
local function sniff(body)
	local n = #body
	if n < 4 then
		return "unknown", "bin"
	end
	local b4 = body:sub(1, 4)
	if b4 == "\x89PNG" then return "image", "png" end
	if body:sub(1, 3) == "\xFF\xD8\xFF" then return "image", "jpg" end
	if b4 == "GIF8" then return "image", "gif" end
	if b4 == "RIFF" and n >= 12 and body:sub(9, 12) == "WEBP" then return "image", "webp" end
	if body:sub(1, 2) == "BM" then return "image", "bmp" end

	if b4 == "OggS" then return "sound", "ogg" end
	if body:sub(1, 3) == "ID3" then return "sound", "mp3" end
	local b2 = body:sub(1, 2)
	if b2 == "\xFF\xFB" or b2 == "\xFF\xF3" or b2 == "\xFF\xF2" then return "sound", "mp3" end
	if b4 == "RIFF" and n >= 12 and body:sub(9, 12) == "WAVE" then return "sound", "wav" end

	if body:sub(1, 8) == "<roblox!" then return "model", "rbxm" end
	if body:sub(1, 7) == "<roblox" then return "model", "rbxmx" end
	if body:sub(1, 8) == "version " then return "mesh", "mesh" end

	if body:sub(1, 5) == "\xABKTX " then return "image", "ktx2" end
	if b4 == "OTTO" or b4 == "\x00\x01\x00\x00" then return "font", "ttf" end

	return "unknown", "bin"
end

-- Accepts either a bare token or a full `name=value; ...` header.
local function normalize_cookie(raw)
	if not raw then return nil end
	raw = raw:gsub("[\r\n]+$", "")
	if raw == "" then return nil end
	if not raw:find("=") then
		raw = ".ROBLOSECURITY=" .. raw
	end
	return raw
end

local function read_cookie()
	if CLI_COOKIE then
		return normalize_cookie(CLI_COOKIE), "--cookie"
	end
	if CLI_COOKIE_FILE then
		if not fs.isFile(CLI_COOKIE_FILE) then
			print("error: --cookie-file path does not exist: " .. CLI_COOKIE_FILE)
			process.exit(1)
		end
		return normalize_cookie(fs.readFile(CLI_COOKIE_FILE)), "--cookie-file " .. CLI_COOKIE_FILE
	end
	local env_token = process.env.ROBLOSECURITY
	if env_token and env_token ~= "" then
		return normalize_cookie(env_token), "$ROBLOSECURITY"
	end
	if fs.isFile(".cookie") then
		return normalize_cookie(fs.readFile(".cookie")), ".cookie"
	end
	return nil, nil
end

local COOKIE, COOKIE_SOURCE = read_cookie()
if COOKIE then
	print(string.format("auth: cookie loaded (%d bytes, from %s)", #COOKIE, COOKIE_SOURCE))
else
	print("auth: no cookie provided; restricted assets will 403")
end

local manifest, err = manifest_lib.read(out_root)
if not manifest then
	print("error: " .. err .. "; run src/export.lua first")
	process.exit(1)
end
local assets = manifest.assets or {}

local ids = util.sorted_keys(assets)
local total = #ids
print(string.format("manifest: %d assets in %s", total, manifest_lib.path(out_root)))

local function http_get(id)
	local url = "https://assetdelivery.roblox.com/v1/asset/?id=" .. id
	local headers = {
		["User-Agent"] = "Roblox/WinInet",
		["Referer"] = "https://www.roblox.com/",
	}
	if COOKIE then
		headers["Cookie"] = COOKIE
	end

	local last_status, last_body
	for attempt = 1, MAX_ATTEMPTS do
		local ok, res = pcall(net.request, { url = url, method = "GET", headers = headers })
		if not ok then
			last_status = -1
			last_body = tostring(res)
		else
			last_status = res.statusCode
			last_body = res.body
			if res.ok then
				return res.statusCode, res.body
			end
			-- 4xx is permanent (auth / missing) — don't retry.
			if res.statusCode >= 400 and res.statusCode < 500 then
				return res.statusCode, res.body
			end
		end
		task.wait(0.5 * attempt)
	end
	return last_status, last_body
end

local function download_one(id, entry)
	if entry.file and fs.isFile(out_root .. "/" .. entry.file) then
		return "skip", entry.kind, entry.file
	end

	local status, body = http_get(id)
	if status < 200 or status >= 300 then
		entry.status = "failed"
		entry.status_code = status
		return "fail", entry.kind, "http " .. tostring(status)
	end

	local sniffed_kind, ext = sniff(body)
	-- Trust the exporter's kind unless it was "unknown" (e.g. a script-source
	-- reference). Animations stay in "animation/" even though they sniff as rbxm.
	local kind = entry.kind
	if kind == nil or kind == "unknown" then
		kind = sniffed_kind
	end

	local rel_path = kind .. "/" .. id .. "." .. ext
	local abs_dir = out_root .. "/" .. kind
	if not fs.isDir(abs_dir) then
		fs.writeDir(abs_dir)
	end
	fs.writeFile(out_root .. "/" .. rel_path, body)

	entry.kind = kind
	entry.file = rel_path
	entry.status = "ok"
	entry.bytes = #body
	return "ok", kind, rel_path
end

local next_idx = 0
local function pull()
	next_idx = next_idx + 1
	return ids[next_idx]
end

local stats = { ok = 0, skip = 0, fail = 0 }
local done = 0

local function worker()
	while true do
		local id = pull()
		if id == nil then
			return
		end
		local entry = assets[id]
		local result, kind, detail = download_one(id, entry)
		stats[result] = stats[result] + 1
		done = done + 1
		print(string.format(
			"[%3d/%3d] %s %-10s %-9s %s",
			done, total,
			result == "fail" and "✗" or (result == "skip" and "·" or "✓"),
			kind or "?", id, detail or ""
		))
	end
end

print(string.format("starting %d concurrent workers...", CONCURRENCY))
for _ = 1, CONCURRENCY do
	task.spawn(worker)
end
while done < total do
	task.wait(0.1)
end

manifest_lib.write(out_root, manifest)
print(string.format("\ndone. ok=%d  skip=%d  fail=%d  total=%d", stats.ok, stats.skip, stats.fail, total))
