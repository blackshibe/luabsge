#include "network.h"
#include <cmath>
#include <cstdlib>

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::add_group(NeuronLayerConfiguration* group) {
    group->index = layers.size();
    layers.push_back(*group);
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::set_weight_config(int from, int to, float weight) {
    configuration.push_back(NeuronConnectionConfiguration {
        from, to, weight, false
    });
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::set_weight_config_positive(int from, int to, float weight) {
    configuration.push_back(NeuronConnectionConfiguration {
        from, to, weight, true
    });
}

// get the starting index of a layer in the flat network array
template <typename Neuron>
int NeuronNetworkConfiguration<Neuron>::layer_start(int layer_index) {
    int start = 0;
    for (int i = 0; i < layer_index; i++) start += layers[i].count;
    return start;
}

// check if there's a connection config between two layers, return weight or 0
template <typename Neuron>
float NeuronNetworkConfiguration<Neuron>::get_connection_weight(int from_layer, int to_layer) {
    for (auto& conn : configuration) {
        if (conn.from == from_layer && conn.to == to_layer) return conn.weight;
    }
    return 0.0f;
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::set_fixed_weight(int from, int to, float weight) {
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

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::build() {
    network_size = 0;
    for (auto& layer : layers) network_size += layer.count;

    network = new Neuron[network_size];

    // zero all weights and eligibility
    for (int i = 0; i < network_size; i++) {
        for (int j = 0; j < network_size; j++) {
            weights_to[i][j] = 0.0f;
            eligibility_to[i][j] = 0.0f;
        }
        trace[i] = 0.0f;
        network[i].network_index = i;
    }

    // assign role to each neuron from its layer
    int cursor = 0;
    for (auto& layer : layers) {
        for (int i = 0; i < layer.count; i++) {
            network[cursor + i].role = layer.role;
        }
        cursor += layer.count;
    }

    float scale = 1.0f / sqrt((float)network_size);

    // initialize weights based on connection configuration
    for (auto& conn : configuration) {
        int from_start = layer_start(conn.from);
        int from_count = layers[conn.from].count;
        int to_start = layer_start(conn.to);
        int to_count = layers[conn.to].count;

        for (int i = 0; i < to_count; i++) {
            for (int j = 0; j < from_count; j++) {
                float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float w = conn.positive_only
                    ? r * scale * conn.weight
                    : (r * 2.0f - 1.0f) * scale * conn.weight;
                weights_to[to_start + i][from_start + j] = w;
            }
        }
    }
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::update(float delta_time) {
    for (int i = 0; i < network_size; i++) {
        network[i].step(network, network_size, delta_time);
    }
}

template <typename Neuron>
Neuron* NeuronNetworkConfiguration<Neuron>::get_in_layer(NeuronLayerConfiguration* layer, int i) {
    return &network[layer_start(layer->index) + i];
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::reward() {
    for (int i = 0; i < network_size; i++) {
        for (int j = 0; j < network_size; j++) {
            weights_to[i][j] += LEARNING_RATE * eligibility_to[i][j];
            weights_to[i][j] = fmax(-1.0f, fmin(1.0f, weights_to[i][j]));
        }
    }
    clear_traces();
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::punish() {
    for (int i = 0; i < network_size; i++) {
        for (int j = 0; j < network_size; j++) {
            weights_to[i][j] -= LEARNING_RATE * eligibility_to[i][j];
            weights_to[i][j] = fmax(-1.0f, fmin(1.0f, weights_to[i][j]));
        }
    }
    clear_traces();
}

template <typename Neuron>
void NeuronNetworkConfiguration<Neuron>::clear_traces() {
    for (int i = 0; i < network_size; i++) {
        trace[i] = 0;
        for (int j = 0; j < network_size; j++) {
            eligibility_to[i][j] = 0.0f;
        }
    }
}

// Explicit template instantiation
template struct NeuronNetworkConfiguration<NeuronStructLIF>;
