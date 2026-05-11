local function set_input(program, name, active)
	for i, v in pairs(program.inputs) do
		if v.label == name then
			v.current = active and 1 or 0
			return
		end
	end
end

local timer = NETWORK_RUNTIME_SECONDS + 0.5
local is_true = true

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
		set_input(BaseTest, "INPUT", is_true)

		if is_next_spike then
			is_true = not is_true
			timer = NETWORK_RUNTIME_SECONDS + 1
		end
	end,
}

return BaseTest
