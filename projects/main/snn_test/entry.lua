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

SNN_PROGRAM = SimonSays

-- forces support for SPEED
function now()
	error("use NETWORK_RUNTIME_SECONDS instead of now()")
end

function reset_sums()
	for i, v in pairs(SNN_PROGRAM.outputs) do
		local output = SNN_Get(SNN_NUM_NEURONS() - i)
		v.sum = 0
	end
end

local time_to_compute = 0
local FIXED_DT = 1 / 100
World.rendering.step:connect(function(delta_time)
	delta_time = delta_time * SPEED

	time_to_compute = time_to_compute + delta_time

	local cutoff = os.clock() + 0.4
	while time_to_compute > FIXED_DT do
		time_to_compute = time_to_compute - FIXED_DT
		fixed_update(FIXED_DT)
		SNN_Update(FIXED_DT)

		if os.clock() > cutoff then
			break
		end
	end

	local dimensions = Window.get_window_dimensions()
	ImGui.SetNextWindowPos(0, 0)
	ImGui.SetNextWindowSize(dimensions.x, dimensions.y)

	local window_flags = ImGui.WindowFlags_NoTitleBar
		+ ImGui.WindowFlags_NoResize
		+ ImGui.WindowFlags_NoMove
		+ ImGui.WindowFlags_NoCollapse
		+ ImGui.WindowFlags_NoBringToFrontOnFocus

	if ImGui.Begin("WINDOW", window_flags) then
		ImGui.Text("Stats")
		ImGui.Text(("Simulation time: %f seconds"):format(NETWORK_RUNTIME_SECONDS))
		ImGui.Text(("Wins: %i, Losses: %i (%f%%)"):format(WINS, LOSSES, (WINS / (WINS + LOSSES)) * 100))
		local _, new_speed = ImGui.SliderFloat("Speed", SPEED, 0.0, 100.0)
		SPEED = new_speed
		ImGui.Separator()

		ImGui.Text("SNN State")

		plot_inputs_for_program(SNN_PROGRAM)
		ImGui.Separator()

		plot_neuron(0)

		ImPlot.PushColormap(ImPlot.Colormap_Pastel)
		for i, v in pairs(SNN_PROGRAM.outputs) do
			local index_of = SNN_NUM_NEURONS() - i
			plot_neuron(index_of, v.label, i ~= 3)
		end
	end
	ImGui.End()
end)

function fixed_update(delta_time)
	NETWORK_RUNTIME_SECONDS = NETWORK_RUNTIME_SECONDS + delta_time

	SNN_PROGRAM.update()

	for i, v in pairs(SNN_PROGRAM.inputs) do
		local input = SNN_Get(i - 1)
		input.is_input = true

		---@diagnostic disable-next-line: unnecessary-if
		if v.current > 0 then
			input:spike()
		end
	end

	for i, v in pairs(SNN_PROGRAM.outputs) do
		local output = SNN_Get(SNN_NUM_NEURONS() - i)

		v.current = output.output
		v.sum = v.sum + output.output
	end

	update_inputs_for_program(SNN_PROGRAM)
	for i, v in pairs(SNN_PROGRAM.outputs) do
		local index_of = SNN_NUM_NEURONS() - i
		update_neuron(index_of)
	end
end
