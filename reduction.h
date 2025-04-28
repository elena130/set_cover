#include <iostream>
#include <vector>
#include <chrono>
#include "setcover.h"

#ifndef REDUCTION_H
#define REDUCTION_H

enum Status {
    FREE = -1,
    FIX_OUT = 0,
    FIX_IN = 1
};

class Reduction {
private:
    unsigned n_rows, n_cols;
    SetCover& sc;
    std::vector<Status> rows;
    std::vector<Status> columns;

    void copy(const Reduction& r);

public:
    Reduction(SetCover& s, const unsigned nr, const unsigned nc);

    ~Reduction();

    Reduction(const Reduction& r);

    void operator=(const Reduction& r);

    void fix_essential_columns();

    void fix_out_dominated_rows();

    void fix_out_dominated_cols();

    void fix_out_heuristic_dom_cols(const std::vector<int>& min_cost_col);

    Status get_row_status(const unsigned i);

    Status get_col_status(const unsigned j);
};

#endif
