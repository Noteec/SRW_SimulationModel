#include "MCS.h"
#include <random>
#include <algorithm>
#include <tuple>
#include <vector>
#include <iostream>
#include <map>

// Функция сортировки двух векторов
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

// Конструктор
MCS::MCS(const std::vector<std::vector<double>>& Q)
    : Q(Q), num_states(Q.size()) {}

std::tuple<std::vector<std::tuple<std::vector<int>, std::vector<double>, std::vector<double>>>, 
           std::tuple<std::vector<int>, std::vector<double>>, 
           std::map<double, double>> 
MCS::modeling(double total_time, int N, double delta_t) {

    std::vector<std::tuple<std::vector<int>, std::vector<double>, std::vector<double>>> realizations; // Добавлено поле памяти
    std::default_random_engine generator(std::random_device{}());

    // Распределения для памяти
    std::uniform_real_distribution<double> active_memory_dist(15.0, 25.0);  // Память активной фазы
    std::uniform_real_distribution<double> passive_memory_dist(3.0, 8.0);   // Память пассивной фазы

    // Начальное состояние процессов
    std::uniform_int_distribution<int> uniform_dist(0, num_states - 1);

    // Симуляция N процессов
    for (int n = 0; n < N; ++n) {
        int current_state = uniform_dist(generator); // Случайное начальное состояние
        std::vector<int> list_of_states = {current_state};
        std::vector<double> times = {0.0};
        std::vector<double> memory_usage; // Для хранения памяти

        // Инициализируем начальную память
        double active_memory = (current_state > 0 ? active_memory_dist(generator) : 0.0);
        double passive_memory = (current_state == 0 ? passive_memory_dist(generator) : 0.0);
        memory_usage.push_back(active_memory + passive_memory);

        double current_time = 0.0;

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

            // Рассчитываем память для текущего состояния
            active_memory = (next_state > 0 ? active_memory_dist(generator) : 0.0);
            passive_memory = (next_state == 0 ? passive_memory_dist(generator) : 0.0);
            memory_usage.push_back(active_memory + passive_memory);

            // Обновляем состояния и времена
            list_of_states.push_back(next_state);
            times.push_back(current_time);

            current_state = next_state;
        }

        // Сохраняем траекторию, включая память
        realizations.emplace_back(list_of_states, times, memory_usage);
    }

    // Объединяем состояния и времена
    std::vector<int> all_states;
    std::vector<double> all_times;

    for (const auto& realization : realizations) {
        const auto& times = std::get<1>(realization);
        const auto& states = std::get<0>(realization);

        all_times.insert(all_times.end(), times.begin(), times.end());
        all_states.insert(all_states.end(), states.begin(), states.end());
    }

    selectionSort(all_times, all_states);

    // Подсчет количества активных процессов и памяти
    std::vector<int> active_process_count_at_times(all_times.size(), 0);
    std::map<double, double> memory_usage_over_time;

    for (size_t i = 0; i < all_times.size(); ++i) {
        if (i == 0) {
            active_process_count_at_times[i] = (all_states[i] > 0 ? 1 : 0);
        } else {
            active_process_count_at_times[i] = active_process_count_at_times[i - 1] +
                                               (all_states[i] > 0 ? 1 : -1);
            active_process_count_at_times[i] = std::max(0, active_process_count_at_times[i]);
        }

        int active_processes = active_process_count_at_times[i];
        int passive_processes = N - active_processes;

        double active_memory = active_processes * active_memory_dist(generator);
        double passive_memory = passive_processes * passive_memory_dist(generator);

        double total_memory = active_memory + passive_memory;

        // Добавляем использование памяти для каждого времени
        memory_usage_over_time[all_times[i]] += total_memory; // агрегируем данные по времени
    }

    std::tuple<std::vector<int>, std::vector<double>> process_count_and_times =
        std::make_tuple(active_process_count_at_times, all_times);

    return std::make_tuple(realizations, process_count_and_times, memory_usage_over_time);
}