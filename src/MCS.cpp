#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <map>
#include <algorithm>

struct RequestState {
    double time; 
    int state; 
    double memory;
};

class MarkovSystem {
public:
    using Matrix = std::vector<std::vector<double>>;

    MarkovSystem(const Matrix& Q, double total_time, int num_requests) : Q(Q), total_time(total_time), num_requests(num_requests) {
        if (Q.size() != 2 || Q[0].size() != 2 || Q[1].size() != 2) {
            throw std::invalid_argument("The intensity matrix must be of size 2x2");
        }
    }

    virtual void run() {
        results.resize(num_requests);

        for (int i = 0; i < num_requests; ++i) {
            threads.emplace_back(&MarkovSystem::simulate_request, this, i);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        analyze_results();
    }

protected:
    Matrix Q;
    double total_time;
    int num_requests;
    std::vector<std::thread> threads;
    std::vector<std::vector<RequestState>> results;

    virtual double calculate_memory(int state) = 0;

    void simulate_request(int thread_id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> uniform_dist(0.0, 1.0);

        int current_state = (uniform_dist(gen) < 0.5) ? 0 : 1;
        double current_time = 0.0;

        std::vector<RequestState> local_states;
        double current_memory = calculate_memory(current_state);
        local_states.push_back({current_time, current_state, current_memory});

        while (current_time < total_time) {
            double rate = Q[current_state][1 - current_state];
            std::exponential_distribution<> exp_dist(rate);
            double time_to_next_event = exp_dist(gen);
            current_time += time_to_next_event;

            if (current_time > total_time) break;

            current_state = 1 - current_state;
            current_memory = calculate_memory(current_state);

            local_states.push_back({current_time, current_state, current_memory});
        }

        results[thread_id] = std::move(local_states);
    }

    void syncSort(std::vector<double>& time_points, std::vector<int>& states, std::vector<double>& memory_usage) {
        size_t n = time_points.size();
        std::vector<int> indices(n);
        for (size_t i = 0; i < n; ++i) {
            indices[i] = i;
        }

        std::sort(indices.begin(), indices.end(), [&time_points](int i1, int i2) {
            return time_points[i1] < time_points[i2];
        });

        std::vector<double> sorted_time_points(n);
        std::vector<int> sorted_states(n);
        std::vector<double> sorted_memory(n);

        for (size_t i = 0; i < n; ++i) {
            sorted_time_points[i] = time_points[indices[i]];
            sorted_states[i] = states[indices[i]];
            sorted_memory[i] = memory_usage[indices[i]];
        }

        time_points = std::move(sorted_time_points);
        states = std::move(sorted_states);
        memory_usage = std::move(sorted_memory);
    }

    virtual void analyze_results() {
        std::vector<double> time_points;
        std::vector<int> states;
        std::vector<double> memory_usage;

        for (const auto& request : results) {
            for (const auto& state : request) {
                time_points.push_back(state.time);
                states.push_back(state.state);
                memory_usage.push_back(state.memory);
            }
        }

        syncSort(time_points, states, memory_usage);

        std::vector<int> active_requests_count(time_points.size(), 0);

        for (int i = 0; i < states.size(); ++i) {
            if (i == 0) {
                active_requests_count[i] = (states[i] > 0 ? 1 : 0);
            } else {
                active_requests_count[i] = active_requests_count[i - 1] + (states[i] > 0 ? 1 : -1);
                active_requests_count[i] = std::max(0, active_requests_count[i]);
            }
        }

        std::map<int, std::pair<int, double>> memory_stats;

        for (size_t i = 0; i < time_points.size(); ++i) {
            int active = active_requests_count[i];
            memory_stats[active].first++;
            memory_stats[active].second += memory_usage[i];
        }

        int total_time_steps = time_points.size();
        std::cout << "Probability and average memory usage for k active requests:\n";
        for (const auto& [k, stats] : memory_stats) {
            int count = stats.first;
            double total_memory = stats.second;
            double probability = static_cast<double>(count) / total_time_steps;
            double avg_memory = total_memory / count;
            std::cout << "k = " << k
                      << ", Probability = " << probability
                      << ", Avg. Memory = " << avg_memory << " MB\n";
        }
    }
};

class ContinuousMemoryMarkovSystem : public MarkovSystem {
public:
    ContinuousMemoryMarkovSystem(const Matrix& Q, double total_time, int num_requests,double active_min, double active_max, double inactive_min, double inactive_max)
        : MarkovSystem(Q, total_time, num_requests), active_dist(active_min, active_max), inactive_dist(inactive_min, inactive_max) {}

protected:
    double calculate_memory(int state) override {
        std::random_device rd;
        std::mt19937 gen(rd());
        return (state == 1) ? active_dist(gen) : inactive_dist(gen);
    }

private:
    std::uniform_real_distribution<> active_dist;
    std::uniform_real_distribution<> inactive_dist;
};

class DiscreteMemoryMarkovSystem : public MarkovSystem {
public:
    DiscreteMemoryMarkovSystem(const Matrix& Q, double total_time, int num_requests,
                               const std::vector<double>& active_memory_values,
                               const std::vector<double>& inactive_memory_values,
                               const std::vector<double>& active_probabilities,
                               const std::vector<double>& inactive_probabilities)
        : MarkovSystem(Q, total_time, num_requests),
          active_memory_values(active_memory_values),
          inactive_memory_values(inactive_memory_values),
          active_memory_dist(active_probabilities.begin(), active_probabilities.end()),
          inactive_memory_dist(inactive_probabilities.begin(), inactive_probabilities.end()) {}

protected:
    double calculate_memory(int state) override {
        std::random_device rd;
        std::mt19937 gen(rd());
        if (state == 1) {
            return active_memory_values[active_memory_dist(gen)];
        } else {
            return inactive_memory_values[inactive_memory_dist(gen)];
        }
    }

private:
    std::vector<double> active_memory_values;
    std::vector<double> inactive_memory_values;
    std::discrete_distribution<int> active_memory_dist;
    std::discrete_distribution<int> inactive_memory_dist;
};