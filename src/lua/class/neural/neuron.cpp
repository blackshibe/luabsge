#include "neuron.h"
#include <cstdlib>
#include <vector>

#define NUM_NEURONS 1000
#define SPIKE_POTENTIAL 1.0f
#define LEARNING_RATE 0.001 // anything above 0.0015 is catastrophic

float weights_to[NUM_NEURONS][NUM_NEURONS];
float eligibility_to[NUM_NEURONS][NUM_NEURONS];
float trace[NUM_NEURONS]; // R-STDP learning, eligibility to strengthening value per connection

struct NeuronStruct {
    bool is_input = false;
    int network_index = 0;
    int group_id = -1;
    float loss = 0.0001f;
    float stored = 0.0f;
    float output = 0.0f;

    float threshold = SPIKE_POTENTIAL;
    
    NeuronStruct() {
        threshold = SPIKE_POTENTIAL;
    };

    void step(NeuronStruct* network, int network_size, float timescale) {
        // only non-input neurons take inputs from the other neurons
        float input = 0.0f;

        if (!is_input) {
            for (int j = 0; j < network_size; j++) {
                if (j == network_index) continue;
                input += network[j].output * weights_to[network_index][j];
            }   

            stored += input;
        }

        // spike behavior
        if (stored > threshold) {
            output = 1.0f;
            stored = 0.0f;
            trace[network_index] = 1.0f;
            
            for (int j = 0; j < network_size; j++) {
                if (j == network_index) continue;
                eligibility_to[network_index][j] += trace[j];
            }

            // anti-eligibility: when I fire, weaken eligibility for peers in my group
            for (int k = 0; k < network_size; k++) {
                if (k == network_index || network[k].group_id != group_id) continue;
                for (int j = 0; j < network_size; j++) {
                    eligibility_to[k][j] -= trace[j] * 0.5f;
                }
            }
        } else {
            output = 0.0f;
            trace[network_index] *= pow(0.99f, timescale * 60.0f);
        }

        // after the spike/else block, decay eligibility
        for (int j = 0; j < network_size; j++) {
            eligibility_to[network_index][j] *= pow(0.99f, timescale * 60.0f);
        }

        // leak behavior, bounce down then rebounce
        if (stored > 0.0f) stored *= pow(1 - loss, timescale * 60.0f);
        if (stored < 0.0f) stored = 0.0f; // -stored * 0.1f; // neurons really like going insane
    };

    void spike() {
        stored = threshold + 0.01f;
    }
};

// TODO
// The simon says task doesn't function because the entire brain is interconnected and doesn't know what to do with the reward signals
// There should be a way to define groups of neurons to structure the brain around inputs and outputs, as well as different types (memory neurons have low loss?)
struct NeuronLayerConfiguration {
    int count;
    int index = -1;

    NeuronLayerConfiguration(int count): count(count) {};
};

struct NeuronConnectionConfiguration {
    int from;
    int to;
    float weight;
};

struct NeuronNetworkConfiguration {
    std::vector<NeuronConnectionConfiguration> configuration;
    std::vector<NeuronLayerConfiguration> layers;

    NeuronStruct* network;
    int network_size;

    void add_group(NeuronLayerConfiguration* group) {
        group->index = layers.size();
        layers.push_back(*group);
    }

    void set_weight_config(int from, int to, float weight) {
        configuration.push_back(NeuronConnectionConfiguration {
            from,
            to,
            weight
        });
    }

    // set a fixed uniform weight between all neurons in two layers (e.g. lateral inhibition)
    void set_fixed_weight(int from, int to, float weight) {
        // applied after build(), directly sets weights without randomization
        int from_start = layer_start(from);
        int from_count = layers[from].count;
        int to_start = layer_start(to);
        int to_count = layers[to].count;

        for (int i = 0; i < to_count; i++) {
            for (int j = 0; j < from_count; j++) {
                if (to_start + i == from_start + j) continue; // no self-connection
                weights_to[to_start + i][from_start + j] = weight;
            }
        }
    }

    // get the starting index of a layer in the flat network array
    int layer_start(int layer_index) {
        int start = 0;
        for (int i = 0; i < layer_index; i++) start += layers[i].count;
        return start;
    }

    // check if there's a connection config between two layers, return weight or 0
    float get_connection_weight(int from_layer, int to_layer) {
        for (auto& conn : configuration) {
            if (conn.from == from_layer && conn.to == to_layer) return conn.weight;
        }
        return 0.0f;
    }

    void build() {
        network_size = 0;
        for (auto& layer : layers) network_size += layer.count;

        network = new NeuronStruct[network_size];

        // zero all weights and eligibility
        for (int i = 0; i < network_size; i++) {
            for (int j = 0; j < network_size; j++) {
                weights_to[i][j] = 0.0f;
                eligibility_to[i][j] = 0.0f;
            }
            trace[i] = 0.0f;
            network[i].network_index = i;
        }

        // assign group_id to each neuron
        for (int g = 0; g < (int)layers.size(); g++) {
            int start = layer_start(g);
            for (int i = 0; i < layers[g].count; i++) {
                network[start + i].group_id = g;
            }
        }

        // initialize weights based on connection configuration
        for (auto& conn : configuration) {
            int from_start = layer_start(conn.from);
            int from_count = layers[conn.from].count;
            int to_start = layer_start(conn.to);
            int to_count = layers[conn.to].count;
            float scale = 1.0f / sqrt((float)from_count);

            for (int i = 0; i < to_count; i++) {
                for (int j = 0; j < from_count; j++) {
                    float w = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f) * scale * 3.0 * conn.weight;
                    weights_to[to_start + i][from_start + j] = w;
                }
            }
        }
    }

    void update(float delta_time) {
        for (int i = 0; i < network_size; i++) {
            network[i].step(network, network_size, delta_time);
        }
    }

    NeuronStruct* get_in_layer(NeuronLayerConfiguration* layer, int i) {
        return &network[layer_start(layer->index) + i];
    }

    void reward() {
        for (int i = 0; i < network_size; i++) {
            for (int j = 0; j < network_size; j++) {
                weights_to[i][j] += LEARNING_RATE * eligibility_to[i][j];
                weights_to[i][j] = fmax(-1.0f, fmin(1.0f, weights_to[i][j]));
            }
        }
        for (int i = 0; i < network_size; i++) {
            trace[i] = 0;
            for (int j = 0; j < network_size; j++) {
                eligibility_to[i][j] = 0.0f;
            }
        }
    }

    void punish() {
        for (int i = 0; i < network_size; i++) {
            for (int j = 0; j < network_size; j++) {
                weights_to[i][j] -= LEARNING_RATE * eligibility_to[i][j];
                weights_to[i][j] = fmax(-1.0f, fmin(1.0f, weights_to[i][j]));
            }
        }
        for (int i = 0; i < network_size; i++) {
            trace[i] = 0;
            for (int j = 0; j < network_size; j++) {
                eligibility_to[i][j] = 0.0f;
            }
        }
    }
};



void lua_bsge_init_neuron(sol::state &lua) {
	lua.new_usertype<NeuronNetworkConfiguration>("NeuronNetworkConfiguration",
		sol::constructors<NeuronNetworkConfiguration()>(),
        "add_group", &NeuronNetworkConfiguration::add_group,
        "set_weight_config", &NeuronNetworkConfiguration::set_weight_config,
        "set_fixed_weight", &NeuronNetworkConfiguration::set_fixed_weight,
        "build", &NeuronNetworkConfiguration::build,
        "update", &NeuronNetworkConfiguration::update,
        "get_in_layer", &NeuronNetworkConfiguration::get_in_layer,
        "reward", &NeuronNetworkConfiguration::reward,
        "punish", &NeuronNetworkConfiguration::punish
    );

	lua.new_usertype<NeuronLayerConfiguration>("NeuronLayerConfiguration",
		sol::constructors<NeuronLayerConfiguration(int)>(),
        "index", &NeuronLayerConfiguration::index,
        "count", &NeuronLayerConfiguration::count
    );

	lua.new_usertype<NeuronStruct>("Neuron",
		sol::constructors<NeuronStruct>(),
		"spike", &NeuronStruct::spike,
		"step", &NeuronStruct::step,
		"stored", &NeuronStruct::stored,
		"output", &NeuronStruct::output,
		"threshold", &NeuronStruct::threshold,
		"is_input", &NeuronStruct::is_input
    );
}
