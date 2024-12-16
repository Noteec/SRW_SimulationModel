#include"src/MCS.cpp"
#include <conio.h>

int main() {
    double total_time;
    int num_requests;
    char memory_mode;

    MarkovSystem::Matrix Q = {
        {-1, 1},
        {0.7, -0.7}
    };

    std::cout << "Enter simulation total time: ";
    std::cin >> total_time;
    while (total_time <= 0) {
        std::cerr << "Total simulation time must be positive. Please try again: ";
        std::cin >> total_time;
    }

    std::cout << "Enter request count: ";
    std::cin >> num_requests;
    while (num_requests <= 0) {
        std::cerr << "Number of requests must be positive. Please try again: ";
        std::cin >> num_requests;
    }

    std::cout << "Specify the method of memory calculation (d - sectional, c - continuous): ";
    std::cin >> memory_mode;
    while (memory_mode != 'd' && memory_mode != 'c') {
        std::cerr << "Invalid memory mode. Enter 'd' for discrete or 'c' for continuous: ";
        std::cin >> memory_mode;
    }

    if (memory_mode == 'd') {
        int num_points;
        std::cout << "Enter the number of points in the discrete memory allocation: ";
        std::cin >> num_points;

        std::vector<double> active_memory_values(num_points);
        std::vector<double> inactive_memory_values(num_points);
        std::vector<double> active_probabilities(num_points);
        std::vector<double> inactive_probabilities(num_points);

        for (int i = 0; i < num_points; ++i) {
            std::cout << "Enter the memory value for the point (active state) " << i + 1 << ": ";
            std::cin >> active_memory_values[i];
            std::cout << "Enter the probability for the point (active state) " << i + 1 << ": ";
            std::cin >> active_probabilities[i];
        }

        for (int i = 0; i < num_points; ++i) {
            std::cout << "Enter the memory value for the point (inactive state) " << i + 1 << ": ";
            std::cin >> inactive_memory_values[i];
            std::cout << "Enter the probability for the point (inactive state) " << i + 1 << ": ";
            std::cin >> inactive_probabilities[i];
        }

        DiscreteMemoryMarkovSystem system(Q, total_time, num_requests, active_memory_values, inactive_memory_values, active_probabilities, inactive_probabilities);
        system.run();
    } else {
        double active_min, active_max, inactive_min, inactive_max;
        std::cout << "Enter the minimum and maximum memory value for the active state: ";
        std::cin >> active_min >> active_max;

        std::cout << "Enter the minimum and maximum memory values for the inactive state: ";
        std::cin >> inactive_min >> inactive_max;

        ContinuousMemoryMarkovSystem system(Q, total_time, num_requests, active_min, active_max, inactive_min, inactive_max);
        system.run();
    }

    _getch();

    return 0;
}
