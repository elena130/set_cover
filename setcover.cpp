#include "setcover.h"
#include <iostream>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <set>
#include <cmath>
#include "limits.h"

SetCover::SetCover(unsigned r, unsigned c) : n_rows(r), n_cols(c), rows(r), cols(c), costs(c),
row_density(r, 0), col_density(c), row_assignment(r, FREE), col_assignment(c, FREE) {
    
    for (unsigned i = 0; i < r; ++i) {
        available_row.insert(available_row.end(), i);
    }
    for (unsigned j = 0; j < c; ++j) {
        available_col.insert(available_col.end(), j);
    }
}

SetCover::~SetCover() {
    clear();
}

void SetCover::clear() {
    for (unsigned j = 0; j < n_cols; ++j) {
        Cell* current = cols[j];
        Cell* next;

        for (unsigned k = 0; k < col_density[j]; ++k) {
            next = current->down;
            delete current;
            current = next;
        }
    }

    n_rows = 0;
    n_cols = 0;
    rows.clear();
    cols.clear();
    costs.clear();
    row_density.clear();
    col_density.clear();
    row_assignment.clear();
    col_assignment.clear();
    available_row.clear();
    available_col.clear();
}

void SetCover::copy(const SetCover& s) {
    n_rows = s.n_rows;
    n_cols = s.n_cols;
    rows.resize(n_rows);
    cols.resize(n_cols);
    costs.resize(n_cols);
    row_density.resize(n_rows);
    col_density.resize(n_cols);
    row_assignment.resize(n_rows);
    col_assignment.resize(n_cols);
    available_row = s.available_row;
    available_col = s.available_col;

    for (unsigned j = 0; j < n_cols; j++) {
        costs[j] = s.costs[j];
        col_assignment[j] = s.col_assignment[j];
        Cell* ptr = s.cols[j];
        for (unsigned k = 0; k < s.col_density[j]; ++k) {
            insert_cell(ptr->row, j);
            ptr = ptr->down;
        }
    }

    for (unsigned i = 0; i < n_rows; i++) {
        row_assignment[i] = s.row_assignment[i];
    }
}

SetCover::SetCover(const SetCover& s) {
    copy(s);
}

void SetCover::operator=(const SetCover& s) {
    clear();
    copy(s);
}

Cell* SetCover::column_tail(const unsigned j) {
    if (cols[j] == NULL)
       return NULL;
    else 
       return cols[j]->up;
}

Cell* SetCover::get_col_head(const unsigned j) {
    return cols[j];
}

Cell* SetCover::get_row_head(const unsigned i) {
    return rows[i];
}

void SetCover::insert_cell(const unsigned i, const unsigned j) {
    ++row_density[i];
    ++col_density[j];

    Cell* c = new Cell();
    c->row = i;
    c->col = j;
    Cell* ptr = column_tail(j);
    
    // insertion sort to set the upper and lower pointer 
    if (ptr == NULL) {
        cols[j] = c;
        cols[j]->down = cols[j];
        cols[j]->up = cols[j];
    }
    else {
        while (ptr->row > i && ptr != cols[j]) {
            ptr = ptr->up;
        }

        if (ptr == cols[j] && ptr->row > i) {
            Cell* prec = ptr->up;
            c->up = prec;
            prec->down = c;
            ptr->up = c;
            c->down = ptr;
            cols[j] = c;
        }
        else {
            Cell* next = ptr->down;
            ptr->down = c;
            c->up = ptr;
            c->down = next;
            next->up = c;
        }
        
    }

    // insertion sort to set right and left pointer 
    if (rows[i] == NULL) {
        rows[i] = c;
        c->right = c;
        c->left = c;
    }
    else {
        ptr = rows[i]->left;

        while (ptr->col > j && ptr != rows[i]) {
            ptr = ptr->left;
        }

        if (ptr == rows[i] && ptr->col > j) {
            Cell* prec = ptr->left;
            c->left = prec;
            prec->right = c;
            ptr->left = c;
            c->right = ptr;
            cols[j] = c;
        }
        else {
            Cell* next = ptr->right;
            ptr->right = c;
            c->left = ptr;
            c->right = next;
            next->left = c;
        }
    }
    
}

// check if the set of columns that cover row i is a subset of the columns that cover 
// the row k. 
bool SetCover::row_is_subset_of(const unsigned i, const unsigned k) {
    if (row_density[i] > row_density[k]) 
        return false;

    if(rows[k]->col > rows[i]->col || rows[k]->left->col < rows[i]->left->col) {
        return false;
    }

    Cell* ptr_i = rows[i];
    Cell* ptr_k = rows[k];
    unsigned counter_i = 0;
    unsigned counter_k = 0;

    while( counter_i < row_density[i] && counter_k < row_density[k]) {
        if (ptr_i->col == ptr_k->col) {
            ptr_i = ptr_i->right;
            ++counter_i;
        }
        else if ( ptr_k->col > ptr_i->col){
            return false;
        }

        ptr_k = ptr_k->right;
        ++counter_k;
    }

    return counter_i == row_density[i];
}

// check if a column j is dominated by column k, i.e. when a column j covers a subset of the 
// rows covered by k 
bool SetCover::col_is_dominated(const unsigned j, const unsigned k){
    if (col_density[j] > col_density[k])
        return false;

    if (costs[j] < costs[k])
        return false;

    Cell* ptr_j = cols[j];
    Cell* ptr_k = cols[k];
    unsigned count_j = 0;
    unsigned count_k = 0;

    while (count_k < col_density[k] && count_j < col_density[j]) {
        if (ptr_k->row > ptr_j->row) {
            return false;
        }
        else if (col_density[k] - count_k < col_density[j] - count_j) {
            return false;
        } else if (ptr_k->row == ptr_j->row) {
            ptr_j = ptr_j->down;
            ++count_j;
        }
        ptr_k = ptr_k->down;
        ++count_k;
    }
    return count_j == col_density[j];
}

// removes a row from the set cover, if present. Otherwise it doesn't modify the set cover. 
void SetCover::remove_row(const unsigned i, std::vector<bool>& modified_cols) {
    if (rows[i] == NULL)
        return;

    Cell* ptr = rows[i];
    Cell* prec_cell = rows[i]->up;
    Cell* succ_cell = rows[i]->down;
    Cell* old_ptr;

    // first thing: fix the pointers to deattach the row from the matrix 
    for (unsigned k = 0; k < row_density[i]; ++k) {
        prec_cell->down = succ_cell;
        succ_cell->up = prec_cell;

        if (ptr == get_col_head(ptr->col))
            cols[ptr->col] = ptr->down;

        ptr = ptr->right;
        prec_cell = ptr->up;
        succ_cell = ptr->down;
    }

    // second thing: delete the row and update the metdata to leave the structure consistent 
    ptr = rows[i];
    for (unsigned k = 0; k < row_density[i]; ++k) {
        old_ptr = ptr;
        unsigned j = ptr->col;
        ptr = ptr->right;
        delete old_ptr;

        --col_density[j];
        if (col_density[j] == 0) {
            cols[j] = NULL;
            costs[j] = UINT_MAX;
            available_col.erase(j);
        }
        modified_cols[j] = true;
    }

    rows[i] = NULL;
    row_density[i] = 0;
    available_row.erase(i);
}

// removes col j from the set cover problem, if present. Otherwise it leaves the set cover
// unchanged. 
void SetCover::remove_col(unsigned j, std::vector<bool>& modified_rows) {
    if (cols[j] == NULL)
        return;

    Cell* ptr = cols[j];
    Cell* prec_cell = cols[j]->left;
    Cell* succ_cell = cols[j]->right;

    // fixing the pointers to deattach the column from the matrix 
    for (unsigned k = 0; k < col_density[j]; ++k) {
        prec_cell->right = succ_cell;
        succ_cell->left = prec_cell;

        if (ptr == get_row_head(ptr->row)) {
            rows[ptr->row] = ptr->right;
        }

        ptr = ptr->down;
        prec_cell = ptr->left;
        succ_cell = ptr->right;
    }

    Cell* old_ptr;
    ptr = cols[j];

    for (unsigned k = 0; k < col_density[j]; ++k) {
        old_ptr = ptr;
        unsigned i = ptr->row;
        ptr = ptr->down;

        --row_density[i];
        modified_rows[i] = true;
        if (row_density[i] == 0) {
            rows[i] = NULL;
            available_row.erase(i);
        }

        delete old_ptr;
    }

    cols[j] = NULL;
    col_density[j] = 0;
    available_col.erase(j);
}

void SetCover::chvtal(Solution& chvatal_sol) {
    // number of new rows each column covers if selected in the next iteration 
    std::vector<unsigned> new_cov_rows(n_cols, 0);
    // rows to cover to have a feasible solution
    std::set<unsigned> uncovered_rows(available_row);
    // available cols to select 
    std::set<unsigned> cols_to_select(available_col);
    // cols selected in the solution built by Chvatal
    std::set<unsigned> selected_cols;
    // number of columns which cover each row
    std::vector<unsigned> row_cov_by(n_rows, 0);

    for (unsigned i = 0; i < n_rows; ++i) {
        row_cov_by[i] = 0;
    }

    for (unsigned j = 0; j < n_cols; ++j) {
        new_cov_rows[j] = col_density[j];
    }

    while (!uncovered_rows.empty()) {
        float min_score = UINT_MAX;
        unsigned min_col = 0;

        for (unsigned j : cols_to_select) {
            if (new_cov_rows[j] == 0)
                continue;

            float score = float(costs[j]) / float(new_cov_rows[j]);
            if (score < min_score) {
                min_score = score;
                min_col = j;
            }
        }

        chvatal_sol.add_col(min_col);
        cols_to_select.erase(min_col);

        Cell* col_ptr = cols[min_col];
        // updating the number of new rows each column covers 
        for (unsigned k = 0; k < col_density[min_col]; ++k) {
            Cell* row_ptr = rows[col_ptr->row];

            for (unsigned i = 0; i < row_density[row_ptr->row]; ++i) {
                if (uncovered_rows.find(row_ptr->row) != uncovered_rows.end() && new_cov_rows[row_ptr->col] > 0)
                    --new_cov_rows[row_ptr->col];
                row_ptr = row_ptr->right;
            }
            uncovered_rows.erase(col_ptr->row);
            // everytime a column is added to the solution I update the number of columns which cover the rows 
            ++row_cov_by[col_ptr->row];
            col_ptr = col_ptr->down;
        }

    }

    remove_redundant_cols(chvatal_sol, row_cov_by);
}

void SetCover::remove_redundant_cols(Solution& solution, std::vector<unsigned>& coperte) {
    std::vector<unsigned> ordered_cols(solution.set_s.begin(), solution.set_s.end());
    Cell* ptr;
    bool remove_col;

    // order the columns by their cost, using their index to break ties
    std::sort(ordered_cols.begin(), ordered_cols.end(), [this](unsigned j, unsigned k)->bool {
            if (costs[j] > costs[k]) {
                return true;
            }
            else if (costs[j] == costs[k] && j > k) {
                return true;
            }
            else {
                return false;
            }
        });

    auto iter = ordered_cols.begin();
    while (iter != ordered_cols.end()) {
        unsigned j = *iter;
        remove_col = true;
        ptr = cols[j];

        for (unsigned k = 0; k < col_density[j]; ++k) {
            if (coperte[ptr->row] == 1) {
                remove_col = false;
                break;
            }
            ptr = ptr->down;
        }
        
        if (remove_col) {
            // update the number of columns which cover a row after removing column j 
            Cell* ptr = cols[j];
            for (unsigned k = 0; k < col_density[j]; ++k) {
                --coperte[ptr->row];
                ptr = ptr->down;
            }
            solution.remove_col(j);
        }
        ++iter;
    }
}

// \lambda_i = min_j c_j / |I_j| 
void SetCover::init_multipliers(LagrangianVar& lv) {

    lv.multipliers = std::vector<double>(n_rows, 1000);
    for (unsigned i : available_row) {
        Cell* ptr = rows[i];

        for (unsigned k = 0; k < row_density[i]; ++k) {
            double score = double(costs[ptr->col]) / double(col_density[ptr->col]);
            if (lv.multipliers[i] >= score) {
                lv.multipliers[i] = score;
            }
            ptr = ptr->right;
        }
    }
}

double SetCover::lagrangian_lb(LagrangianPar& lp) {
    LagrangianVar lv;
    lv.solution = std::vector<bool>(n_cols, false);
    lv.subgradients = std::vector<double>(n_rows, 0);
    lv.best_multipliers = lp.init_multipliers;
    lv.cost_lagrang = std::vector<double>(n_cols, 0);
    lv.max_lb = 0;
    lv.lb = 0;
    lv.pi = lp.init_pi;
    lv.t = lp.init_t;
    Solution best_sol;

    unsigned worse_it = 0;      // number of non improving iterations passed
    unsigned max_worse_it = 15; // number of non improving iterations after which pi is updated

    init_multipliers(lv);
    for (unsigned it = 0; it < lp.max_iter && lv.pi > 0.005 && (lp.ub - lv.lb) > lp.min_diff; ++it) {
        lagrangian_solution(lv);
        calc_subgradients(lv);
        update_step_size(lp, lv);
        update_multipliers(lv);

        cost_fixing(lp, lv);

        if (lv.lb > lv.max_lb && lv.lb < lp.ub) {
            lv.max_lb = lv.lb;
            lv.best_multipliers = lv.multipliers;

            Solution feasible_sol = lagrangian_heuristic(lv);
            double feasible_sol_val = 0;
            feasible_sol_val = solution_value(feasible_sol);

            if (feasible_sol_val < lp.ub) {
                lp.ub = feasible_sol_val;
            }

            best_sol = feasible_sol;
            worse_it = 0;
        }
        else {
            ++worse_it;
        }

        if (worse_it == max_worse_it) {
            lv.pi /= 2;
            worse_it = 0;
        }
    }
    return lv.max_lb;
}

void SetCover::cost_fixing(LagrangianPar & lp, LagrangianVar & lv) {

    for (unsigned j : available_col) {
        if (lv.solution[j]) {
            if (lv.lb - lv.cost_lagrang[j] > lp.ub) {
                col_assignment[j] = FIX_IN;
                Cell* ptr = cols[j];
                for (unsigned k = 0; k < col_density[j]; ++k) {
                    row_assignment[ptr->row] = FIX_OUT;
                    ptr = ptr->down;
                }
            }
        }
        else if(lv.lb + lv.cost_lagrang[j] > lp.ub) {
            col_assignment[j] = FIX_OUT;
        }
    }

    // begin reductions 
    std::vector<bool> mod_rows(n_rows, false);
    std::vector<bool> mod_cols(n_cols, false);
    unsigned reduction;
    Logger logger;

    do {
        reduction = 0;
        std::fill_n(mod_rows.begin(), n_rows, false);
        delete_fix_out_cols(mod_rows);
        std::fill_n(mod_cols.begin(), n_cols, false);
        delete_fix_out_rows(mod_cols);

        reduction += fix_essential_columns(false, mod_rows);
        reduction += fix_out_cols_dom_set(false, mod_cols);
        reduction += fix_out_dominated_cols(false, mod_cols, logger);

    } while (reduction != 0);
    

}

// creates a feasible solution for the problem starting out from the lagrangean solution already found. 
Solution SetCover::lagrangian_heuristic(LagrangianVar& lv) {
    // add the minimun cost column to cover all the uncovered rows left by the lagrangean 
    Cell* ptr;
    Solution solution(n_cols);  // create struct to hold the data of the new solution 
    solution.sol = lv.solution;
    std::vector<unsigned> covered_by(n_rows, 0);

    for (unsigned i : available_row) {
        bool covered = false;
        ptr = rows[i];
        unsigned min_cost_col = ptr->col;
        unsigned min_cost = costs[ptr->col];

        for (unsigned k = 0; k < row_density[i]; ++k) {
            if (lv.solution[ptr->col] || solution.sol[ptr->col]) {
                covered = true;
                solution.add_col(ptr->col);
                lv.solution[ptr->col] = true;
                covered_by[i]++;
            }
            if (costs[ptr->col] < min_cost) {
                min_cost_col = ptr->col;
                min_cost = costs[ptr->col];
            }
            ptr = ptr->right;
        }
        if (covered == false) {
            solution.add_col(min_cost_col);
            lv.solution[min_cost_col] = true;
            covered_by[i]++;
        }
    }

    remove_redundant_cols(solution, covered_by);

    return solution;
}

// calculates the lagrangian costs and calculates the solution 
// C_j = c_j - \sum_i \lambda_i * a_ij
// z = \sum_j C_j*x_j + \sum_i \lambda_i
void SetCover::lagrangian_solution(LagrangianVar& lv) {
    lv.lb = 0;
    for (unsigned j : available_col) {
        lv.cost_lagrang[j] = costs[j];
        Cell* ptr = cols[j];
        for (unsigned k = 0; k < col_density[j]; ++k) {
            // \lambda_i * a_ij but a_ij=1
            lv.cost_lagrang[j] -= lv.multipliers[ptr->row];
            ptr = ptr->down;
        }

        if (lv.cost_lagrang[j] <= 0) {
            lv.solution[j] = true;
            lv.lb += lv.cost_lagrang[j];
        }
        else {
            lv.solution[j] = false;
        }
    }

    for (unsigned i : available_row) {
        lv.lb += lv.multipliers[i];
    }
}

// gradients G_i = 1 - \sum_j a_ij * x_j
void SetCover::calc_subgradients(LagrangianVar& lv) {
    for (unsigned i : available_row) {
        // Beasley optimization
        if (std::abs(lv.multipliers[i]) < 0.005 && lv.subgradients[i] < 0) {
            lv.subgradients[i] = 0;
            continue;
        }

        lv.subgradients[i] = 1;
        Cell* ptr = rows[i];
        for (unsigned k = 0; k < row_density[i]; ++k) {
            if(lv.solution[ptr->col])
                lv.subgradients[i] -= 1;
            ptr = ptr->right;
        }
    }
}

// step_size = \phi * (UB - LB) / \sum_i G_i^2
void SetCover::update_step_size(LagrangianPar& lp, LagrangianVar& lv) {
    lv.t = lv.pi * ((1.05*lp.ub) - lv.lb);
    double sum_of_grad = 0;
    for (unsigned i : available_row) {
        sum_of_grad += (lv.subgradients[i] * lv.subgradients[i]);
    }
    lv.t /= sum_of_grad;
}

// updates the value of the multipliers \lambda
// \lambda_i = max(0, \lambda_i + t*G_i)
void SetCover::update_multipliers(LagrangianVar& lv) {
    for (unsigned i : available_row) {
        double updated_val = lv.multipliers[i] + (lv.t * lv.subgradients[i]);
        if (updated_val > 0) {
            lv.multipliers[i] = updated_val;
        }
        else {
            lv.multipliers[i] = 0;
        }
    }
}

bool SetCover::solution_is_correct(const Solution& solution) {
    Cell* ptr;
    bool ok = true;

    for (unsigned i : available_row) {
        ptr = rows[i];
        unsigned counter = 0;

        while (counter < row_density[i] && !solution.sol[ptr->col]) {
            ++counter;
            ptr = ptr->right;
        }

        if (counter == row_density[i]) {
            ok = false;
            std::cout << "Riga: " << ptr->col << "non coperta" << std::endl;
            break;
        }
    }

    return ok;
}

unsigned SetCover::solution_value(const Solution& solution) {
    unsigned solution_cost = 0;

    for (unsigned j : solution.set_s) {
        solution_cost += costs[j];
    }

    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] == FIX_IN)
            solution_cost += costs[j];
    }

    return solution_cost;
}

void SetCover::print_solution(const Solution solution) {
    std::cout << "Columns in solution: ";
    for (unsigned j : solution.set_s) {
        std::cout << j << "\t";
    }
    std::cout << std::endl;
}

void SetCover::set_cost(const unsigned j,  const unsigned cost) {
    costs[j] = cost;
}

unsigned SetCover::get_cost(const unsigned j) {
    return costs[j];
}

unsigned SetCover::get_row_den(const unsigned i){
    return row_density[i];
}

unsigned SetCover::get_col_den(const unsigned j) {
    return col_density[j];
}
