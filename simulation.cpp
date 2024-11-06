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
    int S = 5;      
    int N = 10;      
    double delta_t = 1.0; 

    auto [realizations, process_count_and_times] = model.modeling(start_state, S, N, delta_t);

    // Выводим все реализации
    std::cout << "All Realizations:\n";
    for (int n = 0; n < realizations.size(); ++n) {
        const auto& [states, times] = realizations[n];
        std::cout << "Realization " << n + 1 << ":\n";
        for (size_t i = 0; i < states.size(); ++i) {
            std::cout << "  State: " << states[i] << ", Time: " << times[i] << '\n';
        }
        std::cout << std::endl;
    }

    const auto& [active_process_count_at_times, times] = process_count_and_times;


    // Display active process counts at exact times
    std::cout << "Active Process Counts at Unique Transition Times:\n";
    for (size_t i = N; i < N * S; ++i) {
        std::cout << "Time " << times[i] << ": " 
                  << active_process_count_at_times[i] << " active processes\n";
    }


    std::cin.get();
    return 0;
}
