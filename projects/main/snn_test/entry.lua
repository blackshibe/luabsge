---@diagnostic disable: undefined-global

-- boilerplate
require("bsge")

SPEED = 1
NETWORK_RUNTIME_SECONDS = 0

local SimonSays = require("program.simon_says")
local plot_inputs_mod = require("ui.plot_inputs_for_program")
local update_inputs_for_program = plot_inputs_mod.update_inputs_for_program
local plot_inputs_for_program = plot_inputs_mod.plot_inputs_for_program
local plot_mod = require("ui.plot")
local update_neuron = plot_mod.update_neuron
local plot_neuron = plot_mod.plot_neuron
local network_mod = require("ui.network")
local plot_network = network_mod.plot_network

SNN_PROGRAM = SimonSays

-- forces support for SPEED
function now()
	error("use NETWORK_RUNTIME_SECONDS instead of now()")
end

function reset_sums()
	for i, v in pairs(SNN_PROGRAM.outputs) do
		v.sum = 0
	end
end

SNN_NETWORK = NeuronNetworkConfiguration.new()
local input_layer = NeuronLayerConfiguration.new(8)
-- local hidden_layer = NeuronLayerConfiguration.new(24)
local memory_layer = NeuronLayerConfiguration.new(36)
local output_layer = NeuronLayerConfiguration.new(4)

SNN_NETWORK:add_group(input_layer)
-- SNN_NETWORK:add_group(hidden_layer)
SNN_NETWORK:add_group(memory_layer)
SNN_NETWORK:add_group(output_layer)

SNN_NETWORK:set_weight_config_positive(input_layer.index, memory_layer.index, 3)
-- SNN_NETWORK:set_weight_config(memory_layer.index, hidden_layer.index, 2)
SNN_NETWORK:set_weight_config(memory_layer.index, memory_layer.index, 2)
SNN_NETWORK:set_weight_config(memory_layer.index, output_layer.index, 2)
SNN_NETWORK:set_weight_config(input_layer.index, output_layer.index, 0)
-- SNN_NETWORK:set_weight_config(hidden_layer.index, output_layer.index, 1)

SNN_NETWORK:build()
SNN_NETWORK:set_fixed_weight(input_layer.index, input_layer.index, 0.0)
SNN_NETWORK:set_fixed_weight(output_layer.index, output_layer.index, -1.0)
SNN_NETWORK:set_layer_loss(memory_layer.index, 0.0001) -- memory neurons retain charge much longer

SNN_LAYERS = {
	{
		name = "Input",
		config = input_layer,
		labels = { "TIME", "IN_SEL", "REWARD", "PUNISH", "RED", "BLUE", "GREEN", "YELLOW" },
		layout = "line",
		x = 0,
		y = 0,
	},
	-- { name = "Hidden", config = hidden_layer, x = 2, y = -0.5 },
	{ name = "Memory", config = memory_layer, x = 2, y = 0.5 },
	{
		name = "Output",
		config = output_layer,
		labels = { "RED", "BLUE", "GREEN", "YELLOW" },
		layout = "line",
		x = 3.5,
		y = 0,
	},
}

local time_to_compute = 0
local FIXED_DT = 1 / 100
World.rendering.step:connect(function(delta_time)
	delta_time = delta_time * SPEED

	time_to_compute = time_to_compute + delta_time

	local cutoff = os.clock() + 0.4
	while time_to_compute > FIXED_DT do
		time_to_compute = time_to_compute - FIXED_DT
		fixed_update(FIXED_DT)
		SNN_NETWORK:update(FIXED_DT)

		if os.clock() > cutoff then
			break
		end
	end

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

	-- Dockable window: Stats
	if ImGui.Begin("Stats") then
		ImGui.Text(("Simulation time: %f seconds"):format(NETWORK_RUNTIME_SECONDS))
		ImGui.Text(("Wins: %i, Losses: %i (%f%%)"):format(WINS, LOSSES, (WINS / (WINS + LOSSES)) * 100))
		local _, new_speed = ImGui.SliderFloat("Speed", SPEED, 0.0, 100.0)
		SPEED = new_speed
	end
	ImGui.End()

	-- Dockable window: SNN State
	if ImGui.Begin("SNN State") then
		plot_inputs_for_program(SNN_PROGRAM)
		ImGui.Separator()

		ImPlot.PushColormap(ImPlot.Colormap_Pastel)
		for i, v in pairs(SNN_PROGRAM.outputs) do
			plot_neuron(output_layer.index * 100 + i, v.label, i ~= 3)
		end
		ImPlot.PopColormap()
	end
	ImGui.End()

	-- Dockable window: Network Visualization (no padding)
	ImGui.PushStyleVar(ImGui.StyleVar_WindowPadding, 0, 0)
	if ImGui.Begin("Network Visualization") then
		plot_network()
	end
	ImGui.End()
	ImGui.PopStyleVar()

	-- Dockable window: Test Window
	if ImGui.Begin("Test Window") then
		ImGui.Text("This is a test dockable window!")
		ImGui.Text("You can drag this window by its title bar")
		ImGui.Text("to dock it anywhere in the viewport.")
		ImGui.Separator()
		if ImGui.Button("Click me!") then
			print("Button clicked!")
		end
	end
	ImGui.End()
end)

function fixed_update(delta_time)
	NETWORK_RUNTIME_SECONDS = NETWORK_RUNTIME_SECONDS + delta_time

	SNN_PROGRAM.update()

	for i, v in pairs(SNN_PROGRAM.inputs) do
		local input = SNN_NETWORK:get_in_layer(input_layer, i - 1)
		input.is_input = true

		---@diagnostic disable-next-line: unnecessary-if
		if v.current > 0 then
			input:spike()
		end
	end

	for i, v in pairs(SNN_PROGRAM.outputs) do
		local output = SNN_NETWORK:get_in_layer(output_layer, i - 1)

		v.current = output.output
		v.sum = v.sum + output.stored + output.output
	end

	update_inputs_for_program(SNN_PROGRAM)
	for i, v in pairs(SNN_PROGRAM.outputs) do
		local neuron = SNN_NETWORK:get_in_layer(output_layer, i - 1)
		update_neuron(output_layer.index * 100 + i, neuron)
	end
end
