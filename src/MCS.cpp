#include "MCS.h"
#include <random>
#include <algorithm>
#include <set>
#include <map>

void selectionSort(std::vector<double>& vec1, std::vector<int>& vec2) {
    int n = vec1.size();
    for (int i = 0; i < n - 1; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < n; ++j) {
            if (vec1[j] < vec1[minIndex]) {
                minIndex = j;
            }
        }
        std::swap(vec1[i], vec1[minIndex]);
        std::swap(vec2[i], vec2[minIndex]);
    }
}

MCS::MCS(const std::vector<std::vector<double>>& Q)
    : Q(Q), num_states(Q.size()) {}

std::tuple<std::vector<std::tuple<std::vector<int>, std::vector<double>>>, std::tuple<std::vector<int>, std::vector<double>>> MCS::modeling(int start_state, double total_time, int N, double delta_t) {

    std::vector<std::tuple<std::vector<int>, std::vector<double>>> realizations;

    // Генерация N реализаций
    for (int n = 0; n < N; ++n) {
        int current_state = start_state;
        std::vector<int> list_of_states = { current_state }; 
        std::vector<double> times = { 0.0 };
        double current_time = 0.0; 

        std::default_random_engine generator(std::random_device{}());

        while (current_time < total_time) {
            std::exponential_distribution<double> exp_dist(-Q[current_state][current_state]);
            double time_of_next_state = current_time + exp_dist(generator);

            if (time_of_next_state > total_time) {
                break;
            }

            current_time = time_of_next_state;
            std::vector<double> rates = Q[current_state];
            rates[current_state] = 0;

            std::discrete_distribution<int> state_dist(rates.begin(), rates.end());
            int next_state = state_dist(generator);

            list_of_states.push_back(next_state);
            times.push_back(current_time);

            current_state = next_state;
        }

        realizations.emplace_back(list_of_states, times);
    }


    std::vector<int> all_states;
    std::vector<double> all_times;

    for(const auto& realization : realizations){
        const auto& times = std::get<1>(realization);
        const auto& states = std::get<0>(realization);

        all_times.insert(all_times.end(), times.begin(), times.end());
        all_states.insert(all_states.end(), states.begin(), states.end());
    }

    selectionSort(all_times, all_states);
    
    std::vector<int> active_process_count_at_times(all_times.size(), 0);

    for(int i = N; i < all_times.size(); i++){
        active_process_count_at_times[i] = active_process_count_at_times[i - 1] + (all_states[i] ? 1 : -1);
    }

    std::tuple<std::vector<int>, std::vector<double>> process_count_and_times = std::make_tuple(active_process_count_at_times, all_times);

    return std::make_tuple(realizations, process_count_and_times);
}
