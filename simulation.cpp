#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <tuple>

class Model {
public:
    Model(const std::vector<std::vector<double>>& Q)
        : Q(Q), num_states(Q.size()) {}

    std::tuple<std::vector<int>, std::vector<double>> modeling(int start_state, int S) {    
        int current_state = start_state; // Текущее состояние
        std::vector<int> list_of_states = { current_state }; // Список состояний
        std::vector<double> times = { 0.0 }; // Время переходов состояний
        
        double current_time = 0.0; // Время последнего состояния 
        int s = 0; // Кол-во событий

        std::default_random_engine generator;
        std::exponential_distribution<double> exp_dist;

        while (s < S) {
            exp_dist = std::exponential_distribution<double>(-Q[current_state][current_state]);
            double time_of_next_state = current_time + exp_dist(generator); // Время наступления следующего состояния

            current_time = time_of_next_state;

            std::vector<double> rates = Q[current_state];
            rates[current_state] = 0; // Убираем вероятность оставаться в том же состоянии

            std::discrete_distribution<int> state_dist(rates.begin(), rates.end());
            current_state = state_dist(generator);
            
            list_of_states.push_back(current_state);
            times.push_back(current_time);

            s++;
        }

        return std::make_tuple(list_of_states, times);
    }

private:
    std::vector<std::vector<double>> Q; // Матрица переходов
    int num_states; // Количество состояний
};

int main() {
    // Пример инициализации модели
    std::vector<std::vector<double>> Q = { { -2, 2 }, { 1, -1 } };
    
    Model model(Q);
    auto result = model.modeling(0, 10);
    
    // Обработка результатов
    auto list_of_states = std::get<0>(result);
    auto times = std::get<1>(result);

    // Вывод результатов
    for (int i = 0; i < list_of_states.size(); i++) {
        std::cout << "State: " << list_of_states[i] << "|"<< times[i] << std::endl;
    }
    return 0;
}