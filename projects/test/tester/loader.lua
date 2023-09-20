warn("Running LuaBSGE tests...")

local files = { "image" }

for i, v in pairs(files) do
	local success = require(string.format("tester/test/%s", v))()
	if not success then
		warn("----------------------------------------------------------------------------------------")
		error(string.format("test %s: %s.lua failed!", i, v))
	else
		print(string.format("test passed: %s: %s.lua", i, v))
	end
end

warn("LuaBSGE testing finished")
