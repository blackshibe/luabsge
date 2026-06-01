
local timer = NETWORK_RUNTIME_SECONDS + 0.5
local is_true = true
local frequency = 1.0
local constant_current = false
local current_value = 1.1

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
		if constant_current then
			SetNeuronProgramInputFloat("INPUT", current_value)
		else
			local period = 1.0 / frequency
			local is_next_spike = NETWORK_RUNTIME_SECONDS > timer

			SetNeuronProgramInputFloat("INPUT", is_true and current_value or 0)

			if is_next_spike then
				is_true = not is_true
				timer = NETWORK_RUNTIME_SECONDS + period
			end
		end
	end,

	imgui_update = function()
		local changed_const, new_const = ImGui.Checkbox("Constant Current", constant_current)
		if changed_const then
			constant_current = new_const
		end

		local changed_val, new_val = ImGui.SliderFloat("Current Value", current_value, 0.0, 2.0)
		if changed_val then
			current_value = new_val
		end

		if not constant_current then
			ImGui.Separator()
			local changed_freq, new_freq = ImGui.SliderFloat("Frequency (Hz)", frequency, 0.1, 10.0)
			if changed_freq then
				frequency = new_freq
			end

			ImGui.Text(("Period: %.2f s"):format(1.0 / frequency))
			ImGui.Text(("State: %s"):format(is_true and "ON" or "OFF"))
		end
	end,
}

return BaseTest
