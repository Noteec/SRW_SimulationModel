#include "src/MCS.h"
#include <iostream>

int main() {
    // Определяем матрицу интенсивности переходов
    std::vector<std::vector<double>> Q = {
        {-2, 2},
        {1, -1}
    };
    
    MCS model(Q);
    int start_state = 0;
    int S = 100;      
    int N = 1000;      
    double delta_t = 1.0; 

    auto [realizations, state_count_at_intervals] = model.modeling(start_state, S, N, delta_t);

    // Выводим все реализации
    // std::cout << "All Realizations:\n";
    // for (int n = 0; n < realizations.size(); ++n) {
    //     const auto& [states, times] = realizations[n];
    //     std::cout << "Realization " << n + 1 << ":\n";
    //     for (size_t i = 0; i < states.size(); ++i) {
    //         std::cout << "  State: " << states[i] << ", Time: " << times[i] << '\n';
    //     }
    //     std::cout << std::endl;
    // }

    std::cout << "State '1' distribution over full seconds:\n";
    for (int t = 0; t < state_count_at_intervals.size(); ++t) {
        std::cout << "At second " << t << ": " << state_count_at_intervals[t] << std::endl;
    }

    std::cout << "State '1' distribution over time intervals:\n";
    
    double scaling_factor = 0.2; // Коэффициент масштабирования
    for (int t = 0; t < state_count_at_intervals.size(); ++t) {
        std::cout << "Time interval " << t << ": ";

        int stars_to_display = static_cast<int>(state_count_at_intervals[t] * scaling_factor);
        

        for (int i = 0; i < stars_to_display; ++i) {
            std::cout << "*";
        }

        std::cout << " (" << state_count_at_intervals[t] << ")\n"; 
    }
    std::cin.get();
    return 0;
}
