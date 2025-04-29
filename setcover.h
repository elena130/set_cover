#include <vector>

#ifndef SETCOVER_H
#define SETCOVER_H

struct Cell {
    unsigned row, col;
    Cell* up;
    Cell* right;
    Cell* down;
    Cell* left;
};

enum Status {
    FREE = -1,
    FIX_OUT = 0,
    FIX_IN = 1
};

class SetCover {
private:
    unsigned int n_rows, n_cols;
    std::vector<Cell*> rows;
    std::vector<Cell*> cols;
    std::vector<unsigned> costs;
    std::vector<unsigned> row_density;
    std::vector<unsigned> col_density;
    std::vector<Status> row_assignment;
    std::vector<Status> col_assignment;

    void clear();

    void copy(const SetCover& s);

public:
    SetCover(unsigned r, unsigned c);

    ~SetCover();

    SetCover(const SetCover& s);

    void operator=(const SetCover& s);

    Cell* column_tail(const unsigned j);

    Cell* get_col_head(const unsigned j);

    Cell* get_row_head(const unsigned i);

    void insert_cell(const unsigned i, const unsigned j);

    bool row_is_dominated(const unsigned i, const unsigned k);

    bool col_dominates(const unsigned j, const unsigned k);

    bool col_dom_heuristic(const unsigned j, const std::vector<int> & min_cost_col);

    void remove_row(const unsigned i);

    void remove_col(const unsigned j);

    void set_cost(const unsigned j, const unsigned cost);

    unsigned get_cost(const unsigned j);

    unsigned get_row_den(const unsigned i);

    unsigned get_col_den(const unsigned j);

    void fix_essential_columns();

    void fix_out_dominated_rows();

    void fix_out_dominated_cols();

    void fix_out_heuristic_dom_cols(const std::vector<int>& min_cost_col);

    Status get_row_status(const unsigned i);

    Status get_col_status(const unsigned j);
};

#endif