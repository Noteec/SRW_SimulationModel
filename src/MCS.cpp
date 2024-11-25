#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <atomic>
#include <iomanip>
#include <map>

class MarkovSystem {
public:
    using Matrix = std::vector<std::vector<double>>;

    // Constructor for the class
    MarkovSystem(double lambda, const Matrix& Q, double total_time, int num_requests)
        : lambda(lambda), Q(Q), total_time(total_time), num_requests(num_requests) {
        if (Q.size() != 2 || Q[0].size() != 2 || Q[1].size() != 2) {
            throw std::invalid_argument("The intensity matrix must be of size 2x2");
        }
    }

    // Run simulation
    void run() {
        for (int i = 0; i < num_requests; ++i) {
            threads.emplace_back(&MarkovSystem::simulate_request, this, i + 1);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Calculate and display the probabilities for various k values
        calculate_probability_of_k_active_requests();
    }

private:
    double lambda;                       // Intensity for time between events
    Matrix Q;                            // Transition intensity matrix
    double total_time;                   // Total simulation time
    int num_requests;                    // Number of requests to simulate
    std::vector<std::thread> threads;    // Threads for simulating requests
    std::atomic<int> active_requests{0}; // Tracks active requests (state == 1)
    std::mutex output_mutex;             // Mutex for synchronizing output
    std::map<int, int> active_count_map;  // Map to count the number of active requests
    int total_time_steps = 0;             // Total number of time steps (for probability calculation)

    // Function to simulate a single request
    void simulate_request(int request_id) {
        // Initialize random number generators
        std::random_device rd;
        std::mt19937 gen(rd());
        std::exponential_distribution<> exp_dist(lambda);
        std::uniform_real_distribution<> uniform_dist(0.0, 1.0);

        // Initial state chosen randomly from {0, 1}
        int current_state = (uniform_dist(gen) < 0.5) ? 0 : 1;  // Randomly chosen initial state (0 or 1)

        double current_time = 0.0;

        std::vector<double> times = {current_time};
        std::vector<int> states = {current_state};

        // Increment active request count if state is 1 (active)
        if (current_state == 1) {
            active_requests++;
        }

        while (current_time < total_time) {
            // Adjust lambda based on the number of active requests (state == 1)
            int active = active_requests.load();
            double adjusted_lambda = lambda / active;  // Adjust lambda for active requests
            std::exponential_distribution<> dynamic_exp_dist(adjusted_lambda);  // Reinitialize with adjusted lambda
            double time_to_next_event = dynamic_exp_dist(gen);  // Use dynamically adjusted lambda

            current_time += time_to_next_event;

            if (current_time > total_time) {
                break;
            }

            // Transition probabilities
            double q_out = -Q[current_state][current_state];  // Transition out intensity
            double prob_transition = Q[current_state][1 - current_state] / q_out;

            // Transition between states
            if (uniform_dist(gen) < prob_transition) {
                // State transition
                current_state = 1 - current_state;

                // Update active request count based on the new state
                if (current_state == 1) {
                    active_requests++;  // This request is now active
                } else {
                    active_requests--;  // This request is no longer active
                }
            }

            // Save data for times and states
            times.push_back(current_time);
            states.push_back(current_state);

            // Track the number of active requests at this time step
            active_count_map[active_requests.load()]++;
            total_time_steps++;
        }

        // Output the results for this request
        //print_results(request_id, times, states);
    }

    // Method to output the results of a single request simulation
    void print_results(int request_id, const std::vector<double>& times, const std::vector<int>& states) {
        std::lock_guard<std::mutex> lock(output_mutex); // Protect the output
        std::cout << "Request #" << request_id << "\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Time\tState\n";
        for (size_t i = 0; i < times.size(); ++i) {
            std::cout << times[i] << "\t" << states[i] << "\n";
        }
        std::cout << "-----------------------------\n";
    }

    // Method to calculate and output the probability of having k active requests
    void calculate_probability_of_k_active_requests() {
        std::lock_guard<std::mutex> lock(output_mutex);

        // Calculate the probability for each k active requests
        std::cout << "Probability of k active requests:\n";
        for (int k = 0; k <= num_requests; ++k) {
            int count = active_count_map[k];
            double probability = static_cast<double>(count) / total_time_steps;
            std::cout << "k = " << k << ": " << probability << "\n";
        }
    }
};
