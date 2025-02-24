#include "src/MCS.cpp"
#include <conio.h>
#include <iostream>

int main() {
    double total_time;
    int num_requests;
    char memory_mode;
    RandomGenerator rng;

    MarkovSystem::Matrix Q = {
        {-1, 1},
        {0.7, -0.7}
    };

    std::cout << "Enter simulation total time: ";
    std::cin >> total_time;
    while (total_time <= 0) {
        std::cerr << "Total simulation time must be positive. Try again: ";
        std::cin >> total_time;
    }

    std::cout << "Enter request count: ";
    std::cin >> num_requests;
    while (num_requests <= 0) {
        std::cerr << "Number of requests must be positive. Try again: ";
        std::cin >> num_requests;
    }

    std::cout << "Specify the method of memory calculation (d - discrete, c - continuous): ";
    std::cin >> memory_mode;
    while (memory_mode != 'd' && memory_mode != 'c') {
        std::cerr << "Invalid mode. Enter 'd' for discrete or 'c' for continuous: ";
        std::cin >> memory_mode;
    }
    std::cin.ignore(); // Чтобы избежать проблем с потоком ввода

    if (memory_mode == 'd') {
        int num_points;
        std::cout << "Enter the number of discrete memory points: ";
        std::cin >> num_points;
        while (num_points <= 0) {
            std::cerr << "The number of points must be positive. Try again: ";
            std::cin >> num_points;
        }

        std::vector<double> active_memory_values(num_points);
        std::vector<double> inactive_memory_values(num_points);
        std::vector<double> active_probabilities(num_points);
        std::vector<double> inactive_probabilities(num_points);

        for (int i = 0; i < num_points; ++i) {
            std::cout << "Memory value for active state (point " << i + 1 << "): ";
            std::cin >> active_memory_values[i];

            std::cout << "Probability for active state (point " << i + 1 << "): ";
            std::cin >> active_probabilities[i];
        }

        for (int i = 0; i < num_points; ++i) {
            std::cout << "Memory value for inactive state (point " << i + 1 << "): ";
            std::cin >> inactive_memory_values[i];

            std::cout << "Probability for inactive state (point " << i + 1 << "): ";
            std::cin >> inactive_probabilities[i];
        }

        // Проверка суммы вероятностей
        double active_prob_sum = std::accumulate(active_probabilities.begin(), active_probabilities.end(), 0.0);
        double inactive_prob_sum = std::accumulate(inactive_probabilities.begin(), inactive_probabilities.end(), 0.0);

        if (std::abs(active_prob_sum - 1.0) > 1e-6 || std::abs(inactive_prob_sum - 1.0) > 1e-6) {
            std::cerr << "Error: Probabilities must sum to 1.0! Check your input.\n";
            return 1;
        }

        DiscreteMemoryMarkovSystem system(Q, total_time, num_requests, active_memory_values, inactive_memory_values, active_probabilities, inactive_probabilities, rng);
        system.run();
    } else {
        double active_min, active_max, inactive_min, inactive_max;
        std::cout << "Enter min and max memory values for active state: ";
        std::cin >> active_min >> active_max;

        while (active_max <= active_min) {
            std::cerr << "Error: max memory must be greater than min memory. Try again: ";
            std::cin >> active_min >> active_max;
        }

        std::cout << "Enter min and max memory values for inactive state: ";
        std::cin >> inactive_min >> inactive_max;

        while (inactive_max <= inactive_min) {
            std::cerr << "Error: max memory must be greater than min memory. Try again: ";
            std::cin >> inactive_min >> inactive_max;
        }

        ContinuousMemoryMarkovSystem system(Q, total_time, num_requests, active_min, active_max, inactive_min, inactive_max, rng);
        system.run();
    }

    std::cout << "Press any key to exit...";
    _getch();

    return 0;
}
