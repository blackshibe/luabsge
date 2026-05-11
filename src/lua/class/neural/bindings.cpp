#include "bindings.h"

void lua_bsge_init_neuron(sol::state &lua) {
	lua.new_usertype<NeuronNetworkConfiguration<NeuronStructLIF>>("NeuronNetworkConfigurationLIF",
		sol::constructors<NeuronNetworkConfiguration<NeuronStructLIF>()>(),
        "add_group", &NeuronNetworkConfiguration<NeuronStructLIF>::add_group,
        "set_weight_config", &NeuronNetworkConfiguration<NeuronStructLIF>::set_weight_config,
        "set_weight_config_positive", &NeuronNetworkConfiguration<NeuronStructLIF>::set_weight_config_positive,
        "set_fixed_weight", &NeuronNetworkConfiguration<NeuronStructLIF>::set_fixed_weight,
        "build", &NeuronNetworkConfiguration<NeuronStructLIF>::build,
        "update", &NeuronNetworkConfiguration<NeuronStructLIF>::update,
        "get_in_layer", &NeuronNetworkConfiguration<NeuronStructLIF>::get_in_layer,
        "reward", &NeuronNetworkConfiguration<NeuronStructLIF>::reward,
        "punish", &NeuronNetworkConfiguration<NeuronStructLIF>::punish
    );

    lua["NeuronRole"] = lua.create_table_with( 
     "Input", NeuronRole::Input, 
        "Neuron", NeuronRole::Neuron, 
        "Output", NeuronRole::Output
    );

	lua.new_usertype<NeuronLayerConfiguration>("NeuronLayerConfiguration",
		sol::constructors<NeuronLayerConfiguration(NeuronRole, int)>(),
        "index", &NeuronLayerConfiguration::index,
        "count", &NeuronLayerConfiguration::count,
        "role", &NeuronLayerConfiguration::role
    );

	lua.new_usertype<NeuronStructLIF>("NeuronLIF",
		sol::constructors<NeuronStructLIF>(),
		
        "spike", &NeuronStruct::spike,
		"step", &NeuronStruct::step,

		"output", &NeuronStructLIF::potential,
		"stored", &NeuronStructLIF::potential, // todo remove
		"threshold", &NeuronStructLIF::threshold,
		"role", &NeuronStructLIF::role // todo rename to type
    );


    lua.set_function("SNN_get_weights_to", [](int i, int j)  {
        return weights_to[i][j];
    });

    lua.set_function("SNN_get_eligibility_to", [](int i, int j)  {
        return eligibility_to[i][j];
    });


    
}
