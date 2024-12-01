#include"src/MCS.cpp"

int main() {
    // Параметры симуляции
    double total_time = 100000; // Общее время симуляции
    int num_requests = 15;   // Количество заявок для симуляции

    // Матрица интенсивности переходов Q
    MarkovSystem::Matrix Q = {
        {-3, 3},  // Из состояния 0
        {2, -2}   // Из состояния 1
    };

    // Создание и запуск системы
    MarkovSystem system(Q, total_time, num_requests);
    system.run();

    std::cin.get();

    return 0;
}
