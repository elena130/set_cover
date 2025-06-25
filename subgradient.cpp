#include "setcover.h"
#include <cmath>

LagrangianResult SetCover::lagrangian_lb(LagrangianPar& lp) {
    LagrangianVar lv;
    lv.cost_lagrang = std::vector<double>(n_cols);
    lv.ub = lp.init_ub;    // UB
    lv.lb = 0;  // LB
    lv.pi = lp.init_pi;
    lv.solution = std::vector<bool>(n_cols, false);   // solution vector 
    lv.t = lp.init_t;
    lv.subgradients = std::vector<int>(n_rows);     // G_i
    lv.prec_direction = std::vector<double>(n_rows, 0);
    lv.direction = std::vector<double>(n_rows, 0);
    lv.beta = 0;
    lv.multipliers = std::vector<double>(n_rows, 0);
    lv.worsening_it = 0;

    unsigned max_worsening_it = 15;


    // calculate the initial offset, the costs of all the FIX_IN columns
    unsigned offset = 0;
    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] == FIX_IN)
            offset += costs[j];
    }

    // init structure to store the best found results during iterations 
    LagrangianResult lr;
    lr.ub = lv.ub;
    lr.ub_sol = lp.init_ub_sol;
    lr.lb = 0;
    lr.lb_sol.sol = std::vector<bool>(n_cols, false);

    Solution best_ub;
    double best_lb_value = 0;
    unsigned removed = 0;

    for (unsigned it = 0; it < lp.max_iter && lv.pi > 0.005 && lv.ub != lr.lb;++it) {
        lagrangian_solution(lv);
        lv.lb = lagrangian_sol_value(lv.solution, lv.cost_lagrang, lv.multipliers) + offset;
        calc_subgradients(lv);

        update_beta(lv);
        update_direction(lv);
        update_step_size(lp, lv);

        if ((lv.lb > best_lb_value && std::ceil(lv.lb) <= lr.ub) || removed > 0) {

            Solution ub_sol = lagrangian_heuristic(lv);
            unsigned ub = solution_value(ub_sol);

            if (ub < lr.ub) {
                lv.ub = ub;
                best_ub = ub_sol;

                lr.ub = ub;
                lr.ub_sol = ub_sol;
            }

            if (std::ceil(lv.lb) <= lr.ub && lv.lb > best_lb_value) {
                lv.worsening_it = 0;

                lr.lb = std::ceil(lv.lb);
                lr.lb_sol.sol = lv.solution;
                lr.lagrangian_costs = lv.cost_lagrang;
                lr.multipliers = lv.multipliers;

                best_lb_value = lv.lb;
            }
        }
        else {
            ++lv.worsening_it;
        }

        if (lv.worsening_it == max_worsening_it) {
            lv.pi /= 2.0;
            lv.worsening_it = 0;
        }

        update_multipliers(lp, lv);

        // calculate the costs of the removed elements
        removed = cost_fixing(lp, lv);
        offset += removed;
        // update the best lower bound found
        if (removed > 0) {
            for (unsigned j = 0; j < n_cols; ++j) {
                if (col_assignment[j] != FREE && lr.lb_sol.sol[j]) {
                    lr.lb_sol.remove_col(j);
                }
            }
            best_lb_value = lagrangian_sol_value(lr.lb_sol.sol, lr.lagrangian_costs, lr.multipliers) + offset;
            lr.lb = std::ceil(best_lb_value);
        }
    }

    for (unsigned j = 0; j < n_cols; ++j) {
        if (lr.lb_sol.sol[j] || col_assignment[j] == FIX_IN) {
            lr.lb_sol.add_col(j);
        }
    }

    return lr;
}

// returns the offset given by the fixed columns
unsigned SetCover::cost_fixing(LagrangianPar& lp, LagrangianVar& lv) {
    unsigned offset = 0;
    for (unsigned j : available_col) {
        if (lv.solution[j]) {
            if (lv.lb - lv.cost_lagrang[j] > lv.ub) {
                col_assignment[j] = FIX_IN;
                offset += costs[j];
                // rimetti la colonna 
                lv.solution[j] = true;
                Cell* ptr = cols[j];
                for (unsigned k = 0; k < col_density[j]; ++k) {
                    row_assignment[ptr->row] = FIX_OUT;
                    lv.multipliers[ptr->row] = 0;
                    ptr = ptr->down;
                }
            }
        }
        else if (lv.lb + lv.cost_lagrang[j] > lv.ub) {
            col_assignment[j] = FIX_OUT;
            lv.solution[j] = false;
        }
    }

    // begin reductions 
    std::vector<bool> mod_rows(n_rows, false);
    std::vector<bool> mod_cols(n_cols, false);
    unsigned reduction;
    Logger logger;

    do {

        for (unsigned i = 0; i < n_rows; ++i)
            mod_rows[i] = false;
        delete_fix_out_cols(mod_rows);
        for (unsigned j = 0; j < n_cols; ++j)
            mod_cols[j] = false;
        delete_fix_out_rows(mod_cols);

        reduction = 0;
        reduction += fix_essential_columns(false, mod_rows);
        reduction += fix_out_cols_dom_set(false, mod_cols);
        reduction += fix_out_dominated_cols(false, mod_cols, logger);

        // update the offset, adding the cost of fixed in cols
        for (unsigned j = 0; j < n_cols; ++j) {
            if (col_assignment[j] == FIX_IN && cols[j] != NULL) {
                offset += costs[j];
            }
        }
    } while (reduction != 0);

    return offset;
}

// creates a feasible solution for the problem starting out from the lagrangean solution already found. 
// Suggested by Beasley
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
            if (solution.sol[ptr->col]) {
                covered = true;
                solution.set_s.insert(ptr->col);
                covered_by[i]++;
            }
            // choose the columns which have minimum column cost and lagrangian cost 
            if (costs[ptr->col] < min_cost || (costs[ptr->col] == min_cost && col_density[ptr->col] > col_density[min_cost_col])) {
                min_cost_col = ptr->col;
                min_cost = costs[ptr->col];
            }
            ptr = ptr->right;
        }
        if (covered == false) {
            solution.add_col(min_cost_col);
            // lv.solution[min_cost_col] = true;
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
        }
        else {
            lv.solution[j] = false;
        }
    }
}

// z = \sum_j C_j*x_j + \sum_i \lambda_i
double SetCover::lagrangian_sol_value(const std::vector<bool> solution, const std::vector<double> cost_lagrang, const std::vector<double> multipliers) {
    double solution_value = 0;
    for (unsigned j = 0; j < n_cols; ++j) {
        if (solution[j]) {
            solution_value += cost_lagrang[j];
        }
    }

    for (unsigned i : available_row) {
        solution_value += multipliers[i];
    }
    return solution_value;
}

// gradients G_i = 1 - \sum_j a_ij * x_j
void SetCover::calc_subgradients(LagrangianVar& lv) {
    for (unsigned i : available_row) {
        lv.subgradients[i] = 1;
        Cell* ptr = rows[i];
        for (unsigned k = 0; k < row_density[i]; ++k) {
            if (lv.solution[ptr->col])
                lv.subgradients[i] -= 1;
            ptr = ptr->right;
        }
    }
}

// step_size = \phi * (UB - LB) / \sum_i s_i^2
void SetCover::update_step_size(LagrangianPar& lp, LagrangianVar& lv) {
    double direction_norm_2 = 0;
    for (unsigned i : available_row) {
        direction_norm_2 += (lv.direction[i] * lv.direction[i]);
    }

    if (std::abs(direction_norm_2) < 1e-10) {
        lv.t = 0;
        lv.pi /= 2;
        lv.worsening_it = 0;
    }
    else {
        lv.t = lv.pi * (1.05*(double)lv.ub - lv.lb) / direction_norm_2;
    }
}

// updates the value of the multipliers \lambda
// \lambda_i = max(0, \lambda_i + t*s_i)
void SetCover::update_multipliers(LagrangianPar& lp, LagrangianVar& lv) {

    for (unsigned i : available_row) {
        lv.multipliers[i] = lv.multipliers[i] + (lv.t * lv.direction[i]);

        if (lv.multipliers[i] < 0) {
            lv.multipliers[i] = 0;
        }
    }
}

void SetCover::update_beta(LagrangianVar& lv) {
    // scalar product between direction and subgradient
    double scalar_prod = 0;
    for (unsigned i : available_row) {
        scalar_prod += lv.prec_direction[i] * lv.subgradients[i];
    }

    if (scalar_prod >= 0) {
        lv.beta = 0;
        return;
    }

    double subgradient_norm = std::sqrt(subgradients_sp(lv));

    double prec_direction_norm = 0;
    for (unsigned i : available_row) {
        prec_direction_norm += (lv.prec_direction[i] * lv.prec_direction[i]);
    }
    prec_direction_norm = std::sqrt(prec_direction_norm);

    lv.beta = subgradient_norm / prec_direction_norm;
}

void SetCover::update_direction(LagrangianVar& lv) {
    for (unsigned i : available_row) {
        double dir = (double)lv.subgradients[i] + (lv.beta * lv.prec_direction[i]);
        lv.prec_direction[i] = lv.direction[i];
        lv.direction[i] = dir;
    }
}

unsigned SetCover::subgradients_sp(LagrangianVar& lv) {
    unsigned scalar_prod = 0;
    for (unsigned i : available_row) {
        if (lv.multipliers[i] == 0 && lv.subgradients[i] < 0)
            continue;
        scalar_prod += (lv.subgradients[i] * lv.subgradients[i]);
    }
    return scalar_prod;
}