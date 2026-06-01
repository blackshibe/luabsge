local plot = require("ui.window.plot")

local state = {}

state.name = "State"

function state.update()
	plot.plot_neuron(GetSnnLayerStartIndex(SNN_NETWORK_LAYERS.input), "input neuron")
	plot.plot_neuron(GetSnnLayerStartIndex(SNN_NETWORK_LAYERS.memory), "memory neuron")
	local output_start = GetSnnLayerStartIndex(SNN_NETWORK_LAYERS.output)
	for i, v in pairs(SNN_PROGRAM.outputs) do
		plot.plot_neuron(output_start + (i - 1), v.label, false)
	end
end

return state
