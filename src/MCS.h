#ifndef MCS_H
#define MCS_H

#include <vector>
#include <tuple>

class MCS {
public:
    MCS(const std::vector<std::vector<double>>& Q);
    std::tuple<std::vector<std::tuple<std::vector<int>, std::vector<double>>>, std::vector<int>> 
    modeling(int start_state, int S, int N, double delta_t);

private:
    std::vector<std::vector<double>> Q;
    int num_states;
};

#endif // MCS_H
