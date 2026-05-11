#pragma once

#include "neuron.h"
#include <vector>

// TODO
// The simon says task doesn't function because the entire brain is interconnected and doesn't know what to do with the reward signals
// There should be a way to define groups of neurons to structure the brain around inputs and outputs, as well as different types (memory neurons have low loss?)
struct NeuronLayerConfiguration {
    int count;
    NeuronRole role;
    int index = -1;

    NeuronLayerConfiguration(NeuronRole role, int count): count(count), role(role) {};
};

struct NeuronConnectionConfiguration {
    int from;
    int to;
    float weight;
    bool positive_only;
};

template <typename Neuron = NeuronStruct>
struct NeuronNetworkConfiguration {
    std::vector<NeuronConnectionConfiguration> configuration;
    std::vector<NeuronLayerConfiguration> layers;

    Neuron* network;
    int network_size;

    void add_group(NeuronLayerConfiguration* group);
    void set_weight_config(int from, int to, float weight);
    void set_weight_config_positive(int from, int to, float weight);
    int layer_start(int layer_index);
    float get_connection_weight(int from_layer, int to_layer);
    void set_fixed_weight(int from, int to, float weight);
    Neuron* get_in_layer(NeuronLayerConfiguration* layer, int i);
    void reward();
    void punish();
    void clear_traces();

    void build();
    void update(float delta_time);
};