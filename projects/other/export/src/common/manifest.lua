local fs = require("@lune/fs")
local serde = require("@lune/serde")

local M = {}

function M.path(out_root)
	return out_root .. "/manifest.json"
end

function M.read(out_root)
	local p = M.path(out_root)
	if not fs.isFile(p) then
		return nil, "manifest not found at " .. p
	end
	return serde.decode("json", fs.readFile(p))
end

function M.write(out_root, manifest)
	fs.writeFile(M.path(out_root), serde.encode("json", manifest, true))
end

return M
