local function set_input(program, name, active)
	for i, v in pairs(program.inputs) do
		if v.label == name then
			v.current = active and 1 or 0
			return
		end
	end
end

-- 100ms pulses that mean the game stepped forward
local timer = NETWORK_RUNTIME_SECONDS + 0.1

local BaseTest
BaseTest = {
	name = "BaseTest",

	inputs = {
		{ label = "INPUT", current = 0 },
	},

	outputs = {
		{ label = "OUTPUT", current = 1, sum = 0 },
	},

	update = function()
		local is_next_spike = NETWORK_RUNTIME_SECONDS > timer
		if is_next_spike then
			set_input(BaseTest, "INPUT", true)
			timer = NETWORK_RUNTIME_SECONDS + 1
		else
			set_input(BaseTest, "INPUT", false)
		end
	end,
}

return BaseTest
