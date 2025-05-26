#include <vector>
#include <set>
#include "logger.h"

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

struct Solution {
    bool* sol;
    std::set<unsigned> set_s;

    Solution(const unsigned n_cols) {
        sol = new bool[n_cols]();
    }

    ~Solution() {
        delete[] sol;
    }

    void add_col(const unsigned j) {
        sol[j] = true;
        set_s.insert(j);
    }

    void remove_col(const unsigned j) {
        sol[j] = false;
        set_s.erase(j);
    }
};

class SetCover {
private:
    unsigned int n_rows, n_cols;
    Cell** rows;
    Cell** cols;
    unsigned* costs;
    unsigned* row_density;
    unsigned* col_density;
    Status* row_assignment;
    Status* col_assignment;
    std::set<unsigned> available_row;
    std::set<unsigned> available_col;

    void clear();

    void copy(const SetCover& s);

    void chvatal_reduction(Solution &solution, unsigned* coperte);

    std::vector<Solution> lagrangean_relaxation(LagrangenaPar& lp);

public:
    SetCover(unsigned r, unsigned c);

    ~SetCover();

    SetCover(const SetCover& s);

    void operator=(const SetCover& s);

    Cell* column_tail(const unsigned j);

    Cell* get_col_head(const unsigned j);

    Cell* get_row_head(const unsigned i);

    void insert_cell(const unsigned i, const unsigned j);

    bool row_is_subset_of(const unsigned i, const unsigned k);

    bool col_is_dominated(const unsigned j, const unsigned k);

    void remove_row(const unsigned i, bool* modified_cols);

    void remove_col(const unsigned j, bool* modified_rows);

    void set_cost(const unsigned j, const unsigned cost);

    unsigned get_cost(const unsigned j);

    unsigned get_row_den(const unsigned i);

    unsigned get_col_den(const unsigned j);

    unsigned fix_essential_columns(const bool first_red, bool * modified_rows);

    unsigned fix_out_dominated_rows(const bool first_red, const bool * modified_rows);

    unsigned fix_out_dominated_cols(const bool first_red, bool* modified, const Logger logger);

    unsigned fix_out_cols_dom_set(const bool first_red, const bool* modified);

    void delete_fix_out_rows(bool* modified_cols);

    void delete_fix_out_cols(bool * modified_rows);

    unsigned remaining_rows();

    unsigned remaining_cols();

    void chvtal(Solution & chvatal_sol);

    bool solution_is_correct(const Solution & solution);

    unsigned solution_value(const Solution & solution);
    
    void print_solution(const Solution solution);
};

#endif