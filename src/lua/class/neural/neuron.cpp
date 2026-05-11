#include "neuron.h"

// Global neuron data storage
float weights_to[NUM_NEURONS][NUM_NEURONS];
float eligibility_to[NUM_NEURONS][NUM_NEURONS];
float trace[NUM_NEURONS];

void NeuronStructLIF::step(NeuronStruct* network, int network_size, float timestep) {
    // only non-input neurons take inputs from the other neurons
    // input = 0.0f;
    // if (role != NeuronRole::Input) {
    //     for (int j = 0; j < network_size; j++) {
    //         if (j == network_index) continue;
    //         input += network[j].output * weights_to[network_index][j] * timestep;
    //     }   
    // }


    float newPotential = potential * (1 - timestep/tau_rc) + (timestep/tau_rc * input);
    potential = newPotential;

    // TODO refractory period
    // output = output * (1 - timestep / (loss * 0.1f));

    // float slope = float((input - threshold_potential_Vth)) / membrane_time_constant_Trc;
    // output = threshold_potential_Vth + timescale * slope;
};

void NeuronStructLIF::spike() {
    output = 1.0f;
    potential = 1.0f;
    // stored = membrane_resistance_R + 0.01f;
    // TODO
}

