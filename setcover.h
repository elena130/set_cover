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
    std::vector<bool> sol;
    std::set<unsigned> set_s;

    Solution(const unsigned n_cols) : sol(n_cols, false){}

    void add_col(const unsigned j) {
        sol[j] = true;
        set_s.insert(j);
    }

    void remove_col(const unsigned j) {
        sol[j] = false;
        set_s.erase(j);
    }
};

struct LagrangianPar {
    std::vector<double> init_multipliers;
    double init_pi;         // initial value pi
    double init_t;          // initial value of step size
    unsigned max_iter;      // max number of iterations
    double min_t;           
    double min_diff;        // min difference between ub and lb 
    unsigned ub;
};

struct LagrangianVar {
    std::vector<double> best_multipliers; // best multipliers associatedd with the best lower bound found
    std::vector<double> cost_lagrang;     // lagrangian costs 
    double lb;
    double max_lb;                        // best lb found during iterations
    std::vector<double> multipliers;      // \lambdas 
    double pi;
    std::vector<bool> solution;           // lagrangian solution 
    double t;
    std::vector<double> subgradients;     // G_i
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
    std::vector <Status> col_assignment;
    std::set<unsigned> available_row;
    std::set<unsigned> available_col;

    void clear();

    void copy(const SetCover& s);

    void chvatal_reduction(Solution &solution, std::vector<unsigned>& coperte);

    

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

    void remove_col(unsigned j, std::vector<bool>& modified_rows);

    void set_cost(const unsigned j, const unsigned cost);

    unsigned get_cost(const unsigned j);

    unsigned get_row_den(const unsigned i);

    unsigned get_col_den(const unsigned j);

    unsigned fix_essential_columns(const bool first_red, const std::vector<bool>& modified_rows);

    unsigned fix_out_dominated_rows(const bool first_red, const std::vector<bool>& modified_rows);

    unsigned fix_out_dominated_cols(const bool first_red, const std::vector<bool> & modified, const Logger logger);

    unsigned fix_out_cols_dom_set(const bool first_red, const std::vector<bool> & modified);

    void delete_fix_out_rows(std::vector<bool>& modified_cols);

    void delete_fix_out_cols(std::vector<bool> & modified_rows);

    unsigned remaining_rows();

    unsigned remaining_cols();

    void chvtal(Solution & chvatal_sol);

    double lagrangian_lb(LagrangianPar& lp);

    Solution lagrangian_heuristic(LagrangianVar& lv);

    void lagrangian_solution(LagrangianVar& lv);

    void calc_subgradients(LagrangianVar& lv);

    void update_step_size(LagrangianPar& lp, LagrangianVar& lv);

    void update_multipliers(LagrangianVar& lv);

    bool solution_is_correct(const Solution & solution);

    unsigned solution_value(const Solution & solution);
    
    void print_solution(const Solution solution);
};

#endif