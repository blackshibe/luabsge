local json = require("persistence.json")

local preferences = {
	file = "preferences.json",
	data = {},
}

function preferences.save()
	local file, err = io.open(preferences.file, "w")
	if not file then
		print("Failed to save preferences: " .. tostring(err))
		return false
	end

	file:write(json.encode(preferences.data))
	file:close()
	return true
end

function preferences.load()
	local file = io.open(preferences.file, "r")
	if not file then
		preferences.data = {}
		return false
	end

	local content = file:read("*a")
	file:close()

	local ok, result = pcall(json.decode, content)
	if ok and type(result) == "table" then
		preferences.data = result
		return true
	else
		print("Failed to parse preferences: " .. tostring(result))
		preferences.data = {}
		return false
	end
end

return preferences
