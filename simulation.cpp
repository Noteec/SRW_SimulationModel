#include "src/MCS.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>

int main() {
    // Define the transition intensity matrix
    std::vector<std::vector<double>> Q = {
        {-2, 2},
        {1, -1}
    };

    MCS model(Q);
    double total_time = 1000; // Total simulation time
    int N = 10;              // Number of processes

    // Call modeling function and get results
    auto [realizations, process_count_and_times, memory_usage] = model.modeling(total_time, N, 1.0);
    const auto& [active_process_count_at_times, times] = process_count_and_times;

    // 1. Display detailed realizations for each process
    std::cout << "Detailed Realizations for Each Process:\n";
    for (size_t i = 0; i < realizations.size(); ++i) {
        const auto& [states, event_times, memory] = realizations[i];
        std::cout << "Process " << i + 1 << ":\n";
        for (size_t j = 0; j < states.size(); ++j) {
            std::cout << "  State: " << states[j]
                      << ", Time: " << event_times[j]
                      << ", Memory: " << memory[j] << " MB"
                      << (states[j] > 0 ? " (Active)" : " (Passive)") 
                      << '\n';
        }
        std::cout << std::endl;
    }

    // 2. Display active processes and memory usage at each unique time
    std::cout << "\nMemory Usage and Active Processes at Each Time:\n";
    std::cout << std::setw(10) << "Time"
              << std::setw(20) << "Memory Usage (MB)"
              << std::setw(25) << "Active Processes" << '\n';

    for (size_t i = 0; i < times.size(); ++i) {
        double current_time = times[i];
        int active_processes = active_process_count_at_times[i];
        double memory_at_time = memory_usage.at(current_time);

        std::cout << std::setw(10) << current_time
                  << std::setw(20) << memory_at_time
                  << std::setw(25) << active_processes << '\n';
    }

    // 3. Display the probability distribution of active processes
    std::cout << "\nProbability Distribution of Active Processes:\n";
    std::map<int, int> active_count_histogram;
    for (const auto& active_count : active_process_count_at_times) {
        active_count_histogram[active_count]++;
    }

    std::cout << std::setw(18) << "Active Processes"
              << std::setw(20) << "Probability" << '\n';
    for (const auto& [active_count, count] : active_count_histogram) {
        double probability = static_cast<double>(count) / active_process_count_at_times.size();
        std::cout << std::setw(18) << active_count
                  << std::setw(20) << std::fixed << std::setprecision(4)
                  << probability << '\n';
    }

    std::cin.get();

    return 0;
}
