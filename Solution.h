#include <vector>
#include <set>

#ifndef SOLUTION_H
#define SOLUTION_H

struct Solution {
    std::vector<bool> sol;
    std::set<unsigned> set_s;

    Solution() : sol(), set_s() {}

    Solution(const unsigned n_cols) : sol(n_cols, false) {}

    void add_col(const unsigned j) {
        sol[j] = true;
        set_s.insert(j);
    }

    void remove_col(const unsigned j) {
        sol[j] = false;
        set_s.erase(j);
    }
};

#endif
