#include "neuron.h"
#include <cstdlib>

#define NUM_NEURONS 100
#define SPIKE_POTENTIAL 1.0f
#define LEARNING_RATE 0.001

float weights_to[NUM_NEURONS][NUM_NEURONS];
float eligibility_to[NUM_NEURONS][NUM_NEURONS];
float trace[NUM_NEURONS]; // R-STDP learning, eligibility to strengthening value per connection

struct NeuronStruct {
    bool is_input = false;
    int network_index = 0;
    float loss = 0.0001f;
    float stored = 0.0f;
    float output = 0.0f;

    float threshold = SPIKE_POTENTIAL;
    float target_threshold = SPIKE_POTENTIAL;
    
    NeuronStruct() {
        threshold = SPIKE_POTENTIAL;
    };

    void step(NeuronStruct network[NUM_NEURONS], float timescale) {
        // only non-input neurons take inputs from the other neurons
        float input = 0.0f;

        if (!is_input) {
            for (int j = 0; j < NUM_NEURONS; j++) {
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
            threshold += 0.05f;

            for (int j = 0; j < NUM_NEURONS; j++) {
                if (j == network_index) continue;
                eligibility_to[network_index][j] += trace[j];
            }
        } else {
            output = 0.0f;
            trace[network_index] *= pow(0.99f, timescale * 60.0f);
        }

        // threshold decays back to baseline (continuous)
        threshold += (SPIKE_POTENTIAL - threshold) * 0.1f * timescale;

        // after the spike/else block, decay eligibility
        for (int j = 0; j < NUM_NEURONS; j++) {
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

static NeuronStruct network[NUM_NEURONS];

// TODO
// The simon says task doesn't function because the entire brain is interconnected and doesn't know what to do with the reward signals
// There should be a way to define groups of neurons to structure the brain around inputs and outputs, as well as different types (memory neurons have low loss?)
struct NeuronGroup {
    NeuronStruct* network;
    int count;

    NeuronGroup(int count): count(count) {
        network = new NeuronStruct[count];

        for (int i = 0; i < count; i++) {
            network[i] = NeuronStruct();
        }
    };

    // TODO only update neurons that have spiked
    void step(float delta_time) {
        for (int i = 0; i < NUM_NEURONS; i++) {
            network[i].step(network, delta_time);
        }
    }
};

void lua_bsge_init_neuron(sol::state &lua) {
    for (int i = 0; i < NUM_NEURONS; i++) {
        for (int j = 0; j < NUM_NEURONS; j++) {
            // xavier initialization
            float scale = 1.0f / sqrt((float)NUM_NEURONS);
            weights_to[i][j] = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f) * scale;
            eligibility_to[i][j] = 0.0f;
            trace[i] = 0.0f;
        }

        network[i].network_index = i;
    }


	lua.new_usertype<NeuronStruct>("Neuron",
		sol::constructors<NeuronStruct>(),
		"spike", &NeuronStruct::spike,
		"step", &NeuronStruct::step,
		"stored", &NeuronStruct::stored,
		"output", &NeuronStruct::output,
		"threshold", &NeuronStruct::threshold,
		"is_input", &NeuronStruct::is_input
    );

    lua.set_function("SNN_Get", [](int i)  {
        return &network[i];
    });

    lua.set_function("SNN_NUM_NEURONS", []()  {
        return NUM_NEURONS;
    });

    lua.set_function("SNN_Update", [](float delta_time)  {
        for (int i = 0; i < NUM_NEURONS; i++) {
            network[i].step(network, delta_time);
        }
    });

    

    lua.set_function("SNN_Reward", []()  {
        for (int i = 0; i < NUM_NEURONS; i++) {
            bool i_is_output = i >= NUM_NEURONS - 4;
            for (int j = 0; j < NUM_NEURONS; j++) {
                bool j_is_output = j >= NUM_NEURONS - 4;
                if (i_is_output && j_is_output) continue;
                weights_to[i][j] += LEARNING_RATE * eligibility_to[i][j];
                weights_to[i][j] = fmax(-1.0f, fmin(1.0f, weights_to[i][j]));
            }
        }

        for (int i = 0; i < NUM_NEURONS; i++) {
            trace[i] = 0;
            for (int j = 0; j < NUM_NEURONS; j++) {
                eligibility_to[i][j] = 0.0f;
            }
        }
    });

    lua.set_function("SNN_Punish", []()  {
        for (int i = 0; i < NUM_NEURONS; i++) {
            bool i_is_output = i >= NUM_NEURONS - 4;
            for (int j = 0; j < NUM_NEURONS; j++) {
                bool j_is_output = j >= NUM_NEURONS - 4;
                if (i_is_output && j_is_output) continue;
                weights_to[i][j] -= LEARNING_RATE  * eligibility_to[i][j];
                weights_to[i][j] = fmax(-1.0f, fmin(1.0f, weights_to[i][j]));
            }
        }
        for (int i = 0; i < NUM_NEURONS; i++) {
            trace[i] = 0;
            for (int j = 0; j < NUM_NEURONS; j++) {
                eligibility_to[i][j] = 0.0f;
            }
        }
    });

    lua.set_function("SNN_get_weights_to", [](int i, int j)  {
        return weights_to[i][j];
    });

    lua.set_function("SNN_get_eligibility_to", [](int i, int j)  {
        return trace[i];
    });


    
}
