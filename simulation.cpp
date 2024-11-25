#include"src/MCS.cpp"

int main() {
    // Параметры симуляции
    double lambda = 1;    // Среднее количество заявок в единицу времени
    double total_time = 100000; // Общее время симуляции
    int num_requests = 10;   // Количество заявок для симуляции

    // Матрица интенсивности переходов Q
    MarkovSystem::Matrix Q = {
        {-2, 2},  // Из состояния 0
        {1, -1}   // Из состояния 1
    };

    // Создание и запуск системы
    MarkovSystem system(lambda, Q, total_time, num_requests);
    system.run();

    std::cin.get();

    return 0;
}
