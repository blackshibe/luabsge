#define NUM_NEURONS 1000
#define SPIKE_POTENTIAL 1.0f
#define LEARNING_RATE 0.001

// TODO toss out
extern float weights_to[NUM_NEURONS][NUM_NEURONS];
extern float eligibility_to[NUM_NEURONS][NUM_NEURONS];
extern float trace[NUM_NEURONS]; // R-STDP learning, eligibility to strengthening value per connection

enum NeuronRole {
    Input = 0,
    Neuron = 1,
    Output = 2,
};

struct NeuronStruct {
    // neuron role in network
    NeuronRole role = NeuronRole::Neuron;
    int network_index = 0;

    // output
    float output = 0.0f;
    
    NeuronStruct() {};
    virtual ~NeuronStruct() = default;

    virtual void step(NeuronStruct* network, int network_size, float timescale) = 0;
    virtual void spike() = 0;
};

struct NeuronStructLIF : public NeuronStruct {
    // neuron parameters
    float membrane_resistance_R = 1.0f;
    float threshold_potential_Vth = 1.0f;
    float resting_potential_Vrest = 0.0f;
    float membrane_time_constant_Trc = 0.02f;
    float refractory_period_Tref = 0.002f;
    
    NeuronStructLIF() {
        membrane_resistance_R = 1.0;
    };

    void step(NeuronStruct* network, int network_size, float timescale);
    void spike();
};
