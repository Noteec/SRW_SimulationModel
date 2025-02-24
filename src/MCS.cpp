#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <algorithm>

// Генератор случайных чисел
class RandomGenerator {
public:
    RandomGenerator() : gen(rd()) {}

    double get_uniform_real(double min, double max) {
        std::uniform_real_distribution<> dist(min, max);
        return dist(gen);
    }

    double get_exponential(double rate) {
        std::exponential_distribution<> dist(rate);
        return dist(gen);
    }

    int get_bernoulli(double p) {
        std::bernoulli_distribution dist(p);
        return dist(gen);
    }

    int get_discrete(const std::vector<double>& probabilities) {
        std::discrete_distribution<int> dist(probabilities.begin(), probabilities.end());
        return dist(gen);
    }

private:
    std::random_device rd;
    std::mt19937 gen;
};

// Класс заявки
class Request {
public:
    Request(double initial_time, int initial_state, double initial_memory)
        : time(initial_time), state(initial_state), memory(initial_memory) {}

    void update(double new_time, int new_state, double new_memory) {
        time = new_time;
        state = new_state;
        memory = new_memory;
    }

    double get_time() const { return time; }
    int get_state() const { return state; }
    double get_memory() const { return memory; }

private:
    double time;
    int state;
    double memory;
};

// Базовый класс системы Маркова
class MarkovSystem {
public:
    using Matrix = std::vector<std::vector<double>>;

    MarkovSystem(const Matrix& Q, double total_time, int num_requests, RandomGenerator& rng)
        : Q(Q), total_time(total_time), num_requests(num_requests), rng(rng) {
        if (Q.size() != 2 || Q[0].size() != 2 || Q[1].size() != 2) {
            throw std::invalid_argument("Intensity matrix must be 2x2");
        }
    }

    void run() {
        results.clear();
        for (int i = 0; i < num_requests; ++i) {
            results.push_back(simulate_request());
        }
        analyze_results();
    }

protected:
    Matrix Q;
    double total_time;
    int num_requests;
    RandomGenerator& rng;
    std::vector<std::vector<Request>> results;

    virtual double calculate_memory(int state) = 0;

    // Метод моделирования одной заявки
    std::vector<Request> simulate_request() {
        int current_state = rng.get_bernoulli(0.5);
        double current_time = 0.0;
        double current_memory = calculate_memory(current_state);

        std::vector<Request> states;
        states.emplace_back(current_time, current_state, current_memory);

        while (current_time < total_time) {
            double rate = Q[current_state][1 - current_state];
            double time_to_next_event = rng.get_exponential(rate);
            current_time += time_to_next_event;

            if (current_time > total_time) break;

            current_state = 1 - current_state;
            current_memory = calculate_memory(current_state);
            states.emplace_back(current_time, current_state, current_memory);
        }

        return states;
    }

    void analyze_results() {
        std::vector<double> time_points;
        std::vector<int> states;
        std::vector<double> memory_usage;

        for (const auto& request : results) {
            for (const auto& state : request) {
                time_points.push_back(state.get_time());
                states.push_back(state.get_state());
                memory_usage.push_back(state.get_memory());
            }
        }

        sync_sort(time_points, states, memory_usage);
        calculate_statistics(time_points, states, memory_usage);
    }

    void sync_sort(std::vector<double>& time_points, std::vector<int>& states, std::vector<double>& memory_usage) {
        size_t n = time_points.size();
        std::vector<int> indices(n);
        for (size_t i = 0; i < n; ++i) {
            indices[i] = i;
        }

        std::sort(indices.begin(), indices.end(), [&time_points](int i1, int i2) {
            return time_points[i1] < time_points[i2];
        });

        std::vector<double> sorted_time(n);
        std::vector<int> sorted_states(n);
        std::vector<double> sorted_memory(n);

        for (size_t i = 0; i < n; ++i) {
            sorted_time[i] = time_points[indices[i]];
            sorted_states[i] = states[indices[i]];
            sorted_memory[i] = memory_usage[indices[i]];
        }

        time_points = std::move(sorted_time);
        states = std::move(sorted_states);
        memory_usage = std::move(sorted_memory);
    }

    void calculate_statistics(const std::vector<double>& time_points, const std::vector<int>& states, const std::vector<double>& memory_usage) {
        std::vector<int> active_requests(time_points.size(), 0);
        std::map<int, std::pair<int, double>> memory_stats;

        for (size_t i = 0; i < time_points.size(); ++i) {
            active_requests[i] = (i == 0) ? (states[i] > 0 ? 1 : 0)
                                          : active_requests[i - 1] + (states[i] > 0 ? 1 : -1);
            active_requests[i] = std::max(0, active_requests[i]);

            int active = active_requests[i];
            memory_stats[active].first++;
            memory_stats[active].second += memory_usage[i];
        }

        int total_time_steps = time_points.size();
        std::cout << "Probability and average memory usage for k active requests:\n";
        for (const auto& [k, stats] : memory_stats) {
            double probability = static_cast<double>(stats.first) / total_time_steps;
            double avg_memory = stats.second / stats.first;
            std::cout << "k = " << k
                      << ", Probability = " << probability
                      << ", Avg. Memory = " << avg_memory << " MB\n";
        }
    }
};

// Непрерывная модель памяти
class ContinuousMemoryMarkovSystem : public MarkovSystem {
public:
    ContinuousMemoryMarkovSystem(const Matrix& Q, double total_time, int num_requests,
                                 double active_min, double active_max,
                                 double inactive_min, double inactive_max,
                                 RandomGenerator& rng)
        : MarkovSystem(Q, total_time, num_requests, rng),
          active_dist(active_min, active_max), inactive_dist(inactive_min, inactive_max) {}

protected:
    double calculate_memory(int state) override {
        return (state == 1) ? rng.get_uniform_real(active_dist.a(), active_dist.b())
                            : rng.get_uniform_real(inactive_dist.a(), inactive_dist.b());
    }

private:
    std::uniform_real_distribution<> active_dist;
    std::uniform_real_distribution<> inactive_dist;
};

// Запуск симуляции
int main() {
    RandomGenerator rng;
    MarkovSystem::Matrix Q = {{-0.5, 0.5}, {0.3, -0.3}};
    ContinuousMemoryMarkovSystem system(Q, 100.0, 50, 1.0, 5.0, 0.1, 2.0, rng);
    system.run();
    return 0;
}
