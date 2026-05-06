---@diagnostic disable: undefined-global

-- boilerplate
require("bsge")

SPEED = 1
NETWORK_RUNTIME_SECONDS = 0
SNN_PROGRAM = require("program.base_test")

local update_inputs_for_program = require("ui.plot_inputs_for_program").update_inputs_for_program
local plot_inputs_for_program = require("ui.plot_inputs_for_program").plot_inputs_for_program
local update_neuron = require("ui.plot").update_neuron
local plot_neuron = require("ui.plot").plot_neuron
local plot_network = require("ui.network").plot_network

function reset_sums()
	for _, v in pairs(SNN_PROGRAM.outputs) do
		v.sum = 0
	end
end

-- mirrors NeuronNetworkConfiguration::layer_start: counts layers added before this one
function snn_layer_start(layer)
	local start = 0
	for _, l in pairs(SNN_NETWORK_LAYERS) do
		if l.index < layer.index then
			start = start + l.count
		end
	end

	return start
end

require("snn.config")

local time_to_compute = 0
World.rendering.step:connect(function(delta_time)
	delta_time = delta_time * SPEED
	time_to_compute = time_to_compute + delta_time

	local cutoff = os.clock() + 0.4
	while time_to_compute > FIXED_DT_SECONDS do
		time_to_compute = time_to_compute - FIXED_DT_SECONDS
		fixed_update(FIXED_DT_SECONDS)
		SNN_NETWORK_CONFIG:update(FIXED_DT_SECONDS)

		if os.clock() > cutoff then
			break
		end
	end

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

		-- Push text to the right side
		local text = "SNN Test - 2026/05/06"
		local text_width = ImGui.CalcTextSize(text)
		local available_width = ImGui.GetContentRegionAvail()
		ImGui.SetCursorPosX(ImGui.GetCursorPosX() + available_width - text_width)
		ImGui.Text(text)

		ImGui.EndMainMenuBar()
	end

	if ImGui.Begin("Stats") then
		ImGui.Text(("Simulation time: %f seconds"):format(NETWORK_RUNTIME_SECONDS))
		local _, new_speed = ImGui.SliderFloat("Speed", SPEED, 0.0, 1.0)
		SPEED = new_speed
	end
	ImGui.End()

	if ImGui.Begin("State") then
		plot_inputs_for_program(SNN_PROGRAM)
		plot_neuron(snn_layer_start(SNN_NETWORK_LAYERS.memory), "memory neuron")
		local output_start = snn_layer_start(SNN_NETWORK_LAYERS.output)
		for i, v in pairs(SNN_PROGRAM.outputs) do
			plot_neuron(output_start + (i - 1), v.label, false)
		end
	end
	ImGui.End()

	ImGui.PushStyleVar(ImGui.StyleVar_WindowPadding, 0, 0)
	if ImGui.Begin("Visualization") then
		plot_network()
	end
	ImGui.End()
	ImGui.PopStyleVar()

	ImPlot.PopStyleColor(2)
end)

function fixed_update(delta_time)
	NETWORK_RUNTIME_SECONDS = NETWORK_RUNTIME_SECONDS + delta_time

	SNN_PROGRAM.update()

	for i, v in pairs(SNN_PROGRAM.inputs) do
		local input = SNN_NETWORK_CONFIG:get_in_layer(SNN_NETWORK_LAYERS.input, i - 1)

		---@diagnostic disable-next-line: unnecessary-if
		if v.current > 0 then
			input:spike()
		end
	end

	for i, v in pairs(SNN_PROGRAM.outputs) do
		local output = SNN_NETWORK_CONFIG:get_in_layer(SNN_NETWORK_LAYERS.output, i - 1)

		v.current = output.output
		v.sum = v.sum + output.stored + output.output
	end

	update_inputs_for_program(SNN_PROGRAM)

	for _, v in pairs(SNN_NETWORK_LAYERS) do
		local start = snn_layer_start(v)
		for j = 0, v.count - 1 do
			local neuron = SNN_NETWORK_CONFIG:get_in_layer(v, j)
			update_neuron(start + j, neuron)
		end
	end
end
