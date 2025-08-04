#include <vector>
#include <set>
#include <iterator>
#include "logger.h"
#include "Solution.h"
#include "LagrangianData.h"
#include "status.h"
#include "configuration.h"

#ifndef SETCOVER_H
#define SETCOVER_H

struct Cell {
    unsigned row, col;
    Cell* up;
    Cell* right;
    Cell* down;
    Cell* left;
};

struct LagrangianPar {
    double init_pi;         // initial value pi
    double init_t;          // initial value of step size
    unsigned max_iter;      // max number of iterations
    double min_t;           
    unsigned init_ub;
    Solution init_ub_sol;
};

struct LagrangianVar {
    std::vector<double> cost_lagrang;     // lagrangian costs 
    unsigned ub;
    double lb;
    std::vector<double> multipliers;      // \lambdas 
    double pi;
    std::vector<bool> solution;           // lagrangian solution 
    double t;
    std::vector<int> subgradients;     // G_i
    double beta;
    std::vector<double> direction;
    std::vector<double> prec_direction;
    unsigned worsening_it;
};

// Iteratore costante per righe
class ConstRowIterator {
    const Cell* current;
    const Cell* head;  // per sapere dove fermarci

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const Cell*;
    using difference_type = std::ptrdiff_t;
    using pointer = const Cell**;
    using reference = const Cell*&;

    ConstRowIterator(const Cell* ptr, const Cell* start) : current(ptr), head(start) {}

    const Cell* operator*() const { return current; }

    ConstRowIterator& operator++() {
        if (current && current->right != head)
            current = current->right;
        else
            current = nullptr; // Fine iterazione
        return *this;
    }

    bool operator!=(const ConstRowIterator& other) const {
        return current != other.current;
    }

    bool operator==(const ConstRowIterator& other) const {
        return current == other.current;
    }
};


// Iteratore costante per colonne
class ConstColIterator {
    const Cell* current;
    const Cell* head;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const Cell*;
    using difference_type = std::ptrdiff_t;
    using pointer = const Cell**;
    using reference = const Cell*&;

    explicit ConstColIterator(const Cell* ptr, const Cell* start) : current(ptr), head(start) {}

    const Cell* operator*() const { return current; }

    ConstColIterator& operator++() {
        if (current && current->down != head)
            current = current->down;
        else
            current = nullptr;
        return *this;
    }

    bool operator!=(const ConstColIterator& other) const {
        return current != other.current;
    }

    bool operator==(const ConstColIterator& other) const {
        return current == other.current;
    }
};

// Wrapper per range-based for sulle righe
class ConstRowRange {
    const Cell* head;
public:
    explicit ConstRowRange(const Cell* h) : head(h) {}

    ConstRowIterator begin() const { return ConstRowIterator(head, head); }
    ConstRowIterator end()   const { return ConstRowIterator(nullptr, head); }
};


// Wrapper per range-based for sulle colonne
class ConstColRange {
    const Cell* head;
public:
    explicit ConstColRange(const Cell* h) : head(h) {}

    ConstColIterator begin() const { return ConstColIterator(head, head); }
    ConstColIterator end()   const { return ConstColIterator(nullptr, head); }
};

class SetCover {
private:
    unsigned int n_rows, n_cols;
    std::vector<Cell*> rows;
    std::vector<Cell*> cols;
    std::vector<unsigned> costs;
    std::vector<unsigned> row_density;
    std::vector<unsigned> col_density;
    Configuration conf;
    std::set<unsigned> available_row;
    std::set<unsigned> available_col;

    void clear();

    void copy(const SetCover& s);

    void remove_redundant_cols(Solution &solution, std::vector<unsigned>& coperte);

    unsigned calc_offset();

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

    void print_rows();

    void print_cols();

    unsigned fix_essential_columns(const bool first_red, const std::vector<bool>& modified_rows);

    unsigned fix_out_dominated_rows(const bool first_red, const std::vector<bool>& modified_rows);

    unsigned fix_out_dominated_cols(const bool first_red, const std::vector<bool> & modified, const Logger logger);

    unsigned fix_out_cols_dom_set(const bool first_red, const std::vector<bool> & modified);

    void delete_fix_out_rows(std::vector<bool>& modified_cols);

    void delete_fix_out_cols(std::vector<bool> & modified_rows);

    unsigned remaining_rows();

    unsigned remaining_cols();

    void chvtal(Solution & chvatal_sol, double (*calc_score)(double, double));

    LagrangianResult lagrangian_lb(LagrangianPar& lp, LagrangianVar& lv);

    unsigned cost_fixing(LagrangianPar& lp, LagrangianVar& lv);

    Solution lagrangian_heuristic(LagrangianVar& lv);

    void lagrangian_solution(LagrangianVar& lv);

    double lagrangian_sol_value(const std::vector<bool> solution, const std::vector<double> cost_lagrang, const std::vector<double> multipliers);
    
    void calc_subgradients(LagrangianVar& lv);

    void update_step_size(LagrangianPar& lp, LagrangianVar& lv);

    void update_multipliers(LagrangianPar & lp, LagrangianVar& lv);

    void update_beta(LagrangianVar& lv);

    void update_direction(LagrangianVar& lv);

    unsigned subgradients_sp(LagrangianVar& lv);

    unsigned dynamic_prog(const std::vector<double>& multipliers, unsigned ub, unsigned lb);

    bool solution_is_correct(const Solution & solution);

    unsigned solution_value(const Solution & solution);

    unsigned solution_value_without_fixed_in(const Solution& solution);
    
    void print_solution(const Solution solution);

    const bool can_be_solved();

    unsigned number_of_rows();

    unsigned number_of_cols();

    void change_configuration(Configuration &new_conf);

    const Configuration& get_configuration();

    ConstRowRange row(unsigned i) const;

    ConstColRange col(unsigned j) const;
};



#endif





