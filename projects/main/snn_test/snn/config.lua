SNN_NETWORK_CONFIG = NeuronNetworkConfigurationLIF.new()
SNN_NETWORK_LAYERS = {
	input = NeuronLayerConfiguration.new(NeuronRole.Input, 1),
	memory = NeuronLayerConfiguration.new(NeuronRole.Neuron, 1),
	output = NeuronLayerConfiguration.new(NeuronRole.Output, 1),
}

SNN_NETWORK_CONFIG:add_group(SNN_NETWORK_LAYERS.input)
SNN_NETWORK_CONFIG:add_group(SNN_NETWORK_LAYERS.memory)
SNN_NETWORK_CONFIG:add_group(SNN_NETWORK_LAYERS.output)

SNN_NETWORK_CONFIG:set_weight_config_positive(SNN_NETWORK_LAYERS.input.index, SNN_NETWORK_LAYERS.memory.index, 10)
SNN_NETWORK_CONFIG:set_weight_config_positive(SNN_NETWORK_LAYERS.memory.index, SNN_NETWORK_LAYERS.memory.index, 1)
SNN_NETWORK_CONFIG:set_weight_config_positive(SNN_NETWORK_LAYERS.memory.index, SNN_NETWORK_LAYERS.output.index, 10)

SNN_NETWORK_CONFIG:build()
SNN_NETWORK_CONFIG:set_fixed_weight(SNN_NETWORK_LAYERS.input.index, SNN_NETWORK_LAYERS.input.index, 0.0)
SNN_NETWORK_CONFIG:set_fixed_weight(SNN_NETWORK_LAYERS.output.index, SNN_NETWORK_LAYERS.output.index, -1.0)

SNN_LAYERS = {
	{ name = "Input", config = SNN_NETWORK_LAYERS.input, layout = "line", x = 0, y = 0 },
	{ name = "Memory", config = SNN_NETWORK_LAYERS.memory, x = 2, y = 0 },
	{ name = "Output", config = SNN_NETWORK_LAYERS.output, layout = "line", x = 3.5, y = 0 },
}
