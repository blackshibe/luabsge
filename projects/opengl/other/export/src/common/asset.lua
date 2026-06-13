local M = {}

-- Extracts the numeric asset id from a value that may be either a plain
-- "rbxassetid://N" string or a serialized Content table ({__type, uri, ...}).
function M.id_from_property(prop)
	if prop == nil then
		return nil
	end
	if type(prop) == "string" then
		return prop:match("^rbxassetid://(%d+)$")
	end
	if prop.__type == "Content" and prop.uri then
		return prop.uri:match("^rbxassetid://(%d+)$")
	end
	return nil
end

function M.id_from_uri(uri)
	if type(uri) ~= "string" then
		return nil
	end
	return uri:match("^rbxassetid://(%d+)$")
end

return M
