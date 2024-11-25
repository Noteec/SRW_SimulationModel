#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <atomic>
#include <iomanip>
#include <map>

class MarkovSystem {
public:
    using Matrix = std::vector<std::vector<double>>;

    // Конструктор класса
    MarkovSystem(double lambda, const Matrix& Q, double total_time, int num_requests)
        : lambda(lambda), Q(Q), total_time(total_time), num_requests(num_requests) {
        if (Q.size() != 2 || Q[0].size() != 2 || Q[1].size() != 2) {
            throw std::invalid_argument("Матрица интенсивности должна быть размером 2x2");
        }
    }

    // Запуск симуляции
    void run() {
        for (int i = 0; i < num_requests; ++i) {
            threads.emplace_back(&MarkovSystem::simulate_request, this, i + 1);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // Вывод вероятностей для различных значений k
        calculate_probability_of_k_active_requests();
    }

private:
    double lambda;                       // Интенсивность времени между событиями
    Matrix Q;                            // Матрица интенсивности переходов
    double total_time;                   // Общее время симуляции
    int num_requests;                    // Количество заявок для симуляции
    std::vector<std::thread> threads;    // Потоки для выполнения заявок
    std::atomic<int> active_requests{0}; // Количество активных заявок
    std::mutex output_mutex;             // Мьютекс для синхронизации вывода
    std::map<int, int> active_count_map;  // Карта для подсчета количества активных заявок
    int total_time_steps = 0;             // Общее количество шагов времени

    // Функция для симуляции одной заявки
    void simulate_request(int request_id) {
        // Инициализация генераторов случайных чисел
        std::random_device rd;
        std::mt19937 gen(rd());
        std::exponential_distribution<> exp_dist(lambda);
        std::uniform_real_distribution<> uniform_dist(0.0, 1.0);

        // Начальное состояние случайным образом из {0, 1}
        int current_state = (uniform_dist(gen) < 0.5) ? 0 : 1;  // Равномерное распределение

        double current_time = 0.0;

        std::vector<double> times = {current_time};
        std::vector<int> states = {current_state};

        // Увеличение счетчика активных заявок
        active_requests++;

        while (current_time < total_time) {
            // Учитываем количество активных заявок
            int active = active_requests.load();
            double adjusted_lambda = lambda / active; // Уменьшаем интенсивность в зависимости от нагрузки
            double time_to_next_event = std::exponential_distribution<>(adjusted_lambda)(gen);

            current_time += time_to_next_event;

            if (current_time > total_time) {
                break;
            }

            // Вероятности переходов
            double q_out = -Q[current_state][current_state];
            double prob_transition = Q[current_state][1 - current_state] / q_out;

            // Переход между состояниями
            if (uniform_dist(gen) < prob_transition) {
                current_state = 1 - current_state;
            }

            // Сохранение данных
            times.push_back(current_time);
            states.push_back(current_state);

            // Подсчёт количества активных заявок на каждом шаге времени
            active_count_map[active]++;
            total_time_steps++;
        }

        // Уменьшение счетчика активных заявок
        active_requests--;

        // Вывод результатов
        print_results(request_id, times, states);
    }

    // Метод для вывода результатов заявки
    void print_results(int request_id, const std::vector<double>& times, const std::vector<int>& states) {
        std::lock_guard<std::mutex> lock(output_mutex); // Защита вывода
        std::cout << "Request #" << request_id << "\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Time\tState\n";
        for (size_t i = 0; i < times.size(); ++i) {
            std::cout << times[i] << "\t" << states[i] << "\n";
        }
        std::cout << "-----------------------------\n";
    }

    // Метод для вычисления вероятности нахождения системы с k активными заявками
    void calculate_probability_of_k_active_requests() {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Probability:\n";
        for (int k = 0; k <= num_requests; ++k) {
            // Получаем количество наблюдений для k активных заявок
            int count = active_count_map[k];
            double probability = static_cast<double>(count) / total_time_steps;
            std::cout << "k = " << k << ": " << probability << "\n";
        }
    }
};