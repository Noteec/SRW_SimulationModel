#ifndef MCS_H
#define MCS_H

#include <vector>
#include <tuple>
#include <map>

class MCS {
public:
    MCS(const std::vector<std::vector<double>>& Q);
   std::tuple<std::vector<std::tuple<std::vector<int>, std::vector<double>, std::vector<double>>>, 
           std::tuple<std::vector<int>, std::vector<double>>, 
           std::map<double, double>>  modeling(double total_time, int N, double delta_t);

private:
    std::vector<std::vector<double>> Q;
    int num_states;
};

#endif // MCS_H
