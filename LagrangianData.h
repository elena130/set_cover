#include <vector>
#include "Solution.h"

#ifndef LAGDATA_H
#define LAGDATA_H

struct LagrangianResult {
    std::vector<double> multipliers;
    std::vector<double> lagrangian_costs;
    Solution ub_sol;
    unsigned ub;
    unsigned lb;
    Solution lb_sol;
};

#endif 

