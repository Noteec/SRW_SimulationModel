#include "MCS.h"
#include <random>
#include <algorithm>

MCS::MCS(const std::vector<std::vector<double>>& Q)
    : Q(Q), num_states(Q.size()) {}

std::tuple<std::vector<std::tuple<std::vector<int>, std::vector<double>>>, std::vector<int>> 
MCS::modeling(int start_state, int S, int N, double delta_t) {
    std::vector<std::tuple<std::vector<int>, std::vector<double>>> realizations;

    for (int n = 0; n < N; ++n) {
        int current_state = start_state;
        std::vector<int> list_of_states = { current_state }; 
        std::vector<double> times = { 0.0 };
        double current_time = 0.0; 
        int s = 0; 

        std::default_random_engine generator(std::random_device{}());

        while (s < S) {
            std::exponential_distribution<double> exp_dist(-Q[current_state][current_state]);
            double time_of_next_state = current_time + exp_dist(generator);

            current_time = time_of_next_state;
            std::vector<double> rates = Q[current_state];
            rates[current_state] = 0;

            std::discrete_distribution<int> state_dist(rates.begin(), rates.end());
            current_state = state_dist(generator);

            list_of_states.push_back(current_state);
            times.push_back(current_time);

            s++;
        }

        realizations.emplace_back(list_of_states, times);
    }

    double max_time = 0.0;
    for (const auto& realization : realizations) {
        const auto& [states, times] = realization;
        double last_time = times.back(); 
        max_time = std::max(max_time, last_time);
    }


    int size = static_cast<int>(max_time) + 1;
    std::vector<int> state_count_at_intervals(size, 0); 

    for (const auto& realization : realizations) {
        const auto& [states, times] = realization;
        for (size_t i = 0; i < states.size(); ++i) {
            int interval_index = static_cast<int>(times[i] / delta_t);
            if (states[i] == 1 && interval_index < size) {
                state_count_at_intervals[interval_index]++;
            }
        }
    }

    return std::make_tuple(realizations, state_count_at_intervals);
}
