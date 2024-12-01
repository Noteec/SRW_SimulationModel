#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <map>
#include <algorithm>

struct RequestState {
    double time; // Временная отметка
    int state;   // Состояние заявки (0 или 1)
};

class MarkovSystem {
public:
    using Matrix = std::vector<std::vector<double>>;

    MarkovSystem(const Matrix& Q, double total_time, int num_requests)
        : Q(Q), total_time(total_time), num_requests(num_requests) {
        if (Q.size() != 2 || Q[0].size() != 2 || Q[1].size() != 2) {
            throw std::invalid_argument("The intensity matrix must be of size 2x2");
        }
    }

    void run() {
        results.resize(num_requests); // Подготовка контейнера для результатов

        for (int i = 0; i < num_requests; ++i) {
            threads.emplace_back(&MarkovSystem::simulate_request, this, i);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        analyze_results();
    }

private:
    Matrix Q;  // Матрица интенсивностей
    double total_time;  // Общее время моделирования
    int num_requests;  // Количество заявок
    std::vector<std::thread> threads;
    std::vector<std::vector<RequestState>> results; // Результаты по заявкам

    void simulate_request(int thread_id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> uniform_dist(0.0, 1.0);

        int current_state = (uniform_dist(gen) < 0.5) ? 0 : 1; // Начальное состояние
        double current_time = 0.0;

        std::vector<RequestState> local_states;
        local_states.push_back({current_time, current_state});

        while (current_time < total_time) {
            double rate = Q[current_state][1 - current_state]; 
            std::exponential_distribution<> exp_dist(rate);
            double time_to_next_event = exp_dist(gen);
            current_time += time_to_next_event;

            if (current_time > total_time) break;

            current_state = 1 - current_state;

            local_states.push_back({current_time, current_state});
        }

        results[thread_id] = std::move(local_states);
    }

    void syncSort(std::vector<double>& time_points, std::vector<int>& states) {
        int n = time_points.size();
        
        // Массив индексов от 0 до n-1
        std::vector<int> indices(n);
        for (int i = 0; i < n; ++i) {
            indices[i] = i;
        }

        // Сортировка индексов по значениям в time_points
        std::sort(indices.begin(), indices.end(), [&time_points](int i1, int i2) {
            return time_points[i1] < time_points[i2];
        });

        // Создаем временные отсортированные массивы
        std::vector<double> sorted_time_points(n);
        std::vector<int> sorted_states(n);

        // Используем отсортированные индексы для перестановки элементов в отсортированные массивы
        for (int i = 0; i < n; ++i) {
            sorted_time_points[i] = time_points[indices[i]];
            sorted_states[i] = states[indices[i]];
        }

        // Копируем отсортированные массивы обратно в исходные
        time_points = std::move(sorted_time_points);
        states = std::move(sorted_states);
    }

    void analyze_results() {
        // Массивы для хранения временных меток и состояний всех заявок
        std::vector<double> time_points;
        std::vector<int> states;

        // Заполнение массивов
        for (const auto& request : results) {
            for (const auto& state : request) {
                time_points.push_back(state.time);
                states.push_back(state.state);
            }
        }

        syncSort(time_points, states);

        // Массив для подсчета количества активных заявок в каждый момент времени
        std::vector<int> active_requests_count(time_points.size(), 0);

        // for(int i = 0; i < states.size(); i++){
        //     std::cout << time_points[i] << " : " << states[i] << std::endl;
        // }

        int active_requests = 0;
        // Подсчет количества активных заявок на каждый момент времени
       for (int i = 0; i < states.size(); ++i) {
            if (i == 0) {
                active_requests_count[i] = (states[i] > 0 ? 1 : 0);
            } else {
                active_requests_count[i] = active_requests_count[i - 1] +
                                                (states[i] > 0 ? 1 : -1);
                active_requests_count[i] = std::max(0, active_requests_count[i]);
            }
       }

        // Подсчет частоты встречаемости каждого значения количества активных заявок
        std::map<int, int> active_count_map;
        for (int count : active_requests_count) {
            active_count_map[count]++;
        }

        // Подсчет вероятностей
        int total_time_steps = time_points.size();
        std::cout << "Probability of k active requests:\n";
        for (const auto& [k, count] : active_count_map) {
            double probability = static_cast<double>(count) / total_time_steps;
            std::cout << "k = " << k << ": " << probability << "\n";
        }
    }
};
