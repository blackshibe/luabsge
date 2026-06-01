-- boilerplate
require("bsge")

SPEED = 1
NETWORK_RUNTIME_SECONDS = 0
SNN_PROGRAM = require("program.base_test")

require("snn.config")

local update_inputs_for_program = require("ui.plot_inputs_for_program").update_inputs_for_program
local plot = require("ui.window.plot")
local graphed = require("ui.graphed")
local preferences = require("persistence.prefs")

function ResetSnnSums()
	for _, v in pairs(SNN_PROGRAM.outputs) do
		v.sum = 0
	end
end

-- mirrors NeuronNetworkConfiguration::layer_start: counts layers added before this one
function GetSnnLayerStartIndex(layer)
	local start = 0
	for _, l in pairs(SNN_NETWORK_LAYERS) do
		if l.index < layer.index then
			start = start + l.count
		end
	end

	return start
end

function SetNeuronProgramInputFloat(name, value)
	for i, v in pairs(SNN_PROGRAM.inputs) do
		if v.label == name then
			v.current = value
			return
		end
	end
end

function SetNeuronProgramInputBool(name, value)
	for i, v in pairs(SNN_PROGRAM.inputs) do
		if v.label == name then
			v.current = value and 1 or 0
			return
		end
	end
end

UI_TABS = {
	inputs = require("ui.window.inputs"),
	stats = require("ui.window.stats"),
	state = require("ui.window.state"),
	visualization = require("ui.window.visualization"),
	speed = require("ui.window.speed"),
	program = require("ui.window.program"),
}


-- Initialize window open states from preferences
preferences.data.windows = preferences.data.windows or {}
for key, tab in pairs(UI_TABS) do
	if preferences.data.windows[key] == nil then
		preferences.data.windows[key] = true
	end
	tab.open = preferences.data.windows[key]
end


local time_to_compute = 0
World.rendering.step:connect(function(delta_time)
	delta_time = delta_time * SPEED
	time_to_compute = time_to_compute + delta_time

	-- fixed timestep compute
	local cutoff = os.clock() + 0.4
	while time_to_compute > FIXED_DT_SECONDS do
		time_to_compute = time_to_compute - FIXED_DT_SECONDS
		FixedUpdate(FIXED_DT_SECONDS)
		SNN_NETWORK_CONFIG:update(FIXED_DT_SECONDS)

		if os.clock() > cutoff then
			break
		end
	end

	-- update UI
	ImPlot.PushStyleColor(ImPlot.Col_FrameBg, 0.1, 0.1, 0.1, 0.0)
	ImPlot.PushStyleColor(ImPlot.Col_PlotBg, 0.04, 0.04, 0.04, 1.0)

	-- Main menu bar
	if ImGui.BeginMainMenuBar() then
		if ImGui.BeginMenu("File") then
			-- ImGui.Separator()
			if ImGui.MenuItem("Exit") then
				os.exit()
			end
			ImGui.EndMenu()
		end

		if ImGui.BeginMenu("Window") then
			for key, tab in pairs(UI_TABS) do
				if ImGui.MenuItem(tab.name, "", tab.open) then
					tab.open = not tab.open

					preferences.data.windows[key] = tab.open
					preferences.save()
				end
			end
			ImGui.EndMenu()
		end

		-- Push text to the right side
		local text = "SNN Test - 2026/05/06"
		local text_width = ImGui.CalcTextSize(text)
		local available_width = ImGui.GetContentRegionAvail()
		ImGui.SetCursorPosX(ImGui.GetCursorPosX() + available_width - text_width)
		ImGui.Text(text)

		ImGui.EndMainMenuBar()
	end

	-- Windows
	for key, tab in pairs(UI_TABS) do
		if tab.open and tab.update then
			if tab.push_styles then tab.push_styles() end
			local open, visible = ImGui.Begin(tab.name, true)
			if visible then
				tab.update()
			end
			ImGui.End()
			if tab.pop_styles then tab.pop_styles() end
			if not open then
				tab.open = false
				preferences.data.windows[key] = false
				preferences.save()
			end
		end
	end

	-- Graphed neuron windows
	local to_remove = nil
	for i, g in ipairs(graphed.neurons) do
		local title = ("%s #%d###graphed_%d"):format(g.name, g.j, i)
		ImGui.SetNextWindowSize(400, 350, ImGui.Cond_FirstUseEver)
		ImGui.PushStyleVar(ImGui.StyleVar_WindowPadding, 0, 0)
		local open, visible = ImGui.Begin(title, true)
		if visible then
			plot.plot_neuron(g.global, nil, false, ImPlot.Flags_NoInputs + ImPlot.Flags_NoMenus + ImPlot.Flags_NoTitle)
		end
		ImGui.End()
		ImGui.PopStyleVar()
		if not open then
			to_remove = i
		end
	end

	if to_remove then
		table.remove(graphed.neurons, to_remove)
		preferences.save()
	end

	ImPlot.PopStyleColor(2)
end)

function FixedUpdate(delta_time)
	NETWORK_RUNTIME_SECONDS = NETWORK_RUNTIME_SECONDS + delta_time

	SNN_PROGRAM.update()

	for i, v in pairs(SNN_PROGRAM.inputs) do
		local input = SNN_NETWORK_CONFIG:get_in_layer(SNN_NETWORK_LAYERS.input, i - 1)

		input.input = v.current
	end

	for i, v in pairs(SNN_PROGRAM.outputs) do
		local output = SNN_NETWORK_CONFIG:get_in_layer(SNN_NETWORK_LAYERS.output, i - 1)

		v.current = output.output
		v.sum = v.sum + output.stored + output.output
	end

	update_inputs_for_program(SNN_PROGRAM)
	plot.always_update()
end
