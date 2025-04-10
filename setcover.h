#include <vector>
#include "cell.h"

#ifndef SETCOVER_H
#define SETCOVER_H

class SetCover {
private:
    unsigned int n_rows, n_cols;
    std::vector<Cell*> rows;
    std::vector<Cell*> cols;
    std::vector<unsigned> costs;
    std::vector<unsigned> row_density;
    std::vector<unsigned> col_density;

public:
    SetCover(int r, int c);

    ~SetCover();

    SetCover(const SetCover& s);

    void insert_cell(const unsigned i, const unsigned j);

    void set_cost(const unsigned j, const unsigned cost);

    void inc_row_den(const unsigned i);

    void set_col_den(const unsigned j, const unsigned den);

    unsigned get_col_den(const unsigned j);
};

#endif