#include <vector>
#include <set>

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
    std::set<unsigned> available_row;
    std::set<unsigned> available_col;

    void clear();

    void copy(const SetCover& s);

    void chvatal_reduction(std::set<unsigned>& selected_cols, std::vector<unsigned>& coperte);

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

    void remove_row(const unsigned i, std::vector<bool>& modified_cols);

    void remove_col(const unsigned j, std::vector<bool> & modified_rows);

    void set_cost(const unsigned j, const unsigned cost);

    unsigned get_cost(const unsigned j);

    unsigned get_row_den(const unsigned i);

    unsigned get_col_den(const unsigned j);

    unsigned fix_essential_columns(const bool first_red, std::vector<bool> modified_rows);

    unsigned fix_out_dominated_rows(const bool first_red, const std::vector<bool>& modified_rows);

    unsigned fix_out_dominated_cols(const bool first_red, std::vector<bool> modified, const bool show_prints);

    unsigned fix_out_cols_dom_set(const bool first_red, const std::vector<bool> & modified);

    void delete_fix_out_rows(std::vector<bool>& modified_cols);

    void delete_fix_out_cols(std::vector<bool> & modified_rows);

    unsigned remaining_rows();

    unsigned remaining_cols();

    void chvtal();

    bool solution_is_correct(const SetCover original);

    unsigned solution_value(const SetCover original);
    
    void print_solution();
};

#endif