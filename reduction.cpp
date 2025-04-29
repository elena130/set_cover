#include <iostream>
#include <chrono>
#include "setcover.h"

void SetCover::fix_essential_columns() {
    unsigned fixed_cols = 0;
    for (unsigned i = 0; i < n_rows; ++i) {
        if (row_density[i] == 1) {
            col_assignment[get_row_head(i)->col] = FIX_IN;
            ++fixed_cols;
        }
    }

    std::cout << "Fixed columns: " << fixed_cols << std::endl;
}

void SetCover::fix_out_dominated_rows() {
    unsigned dominated_rows = 0;

    for (unsigned i = 0; i < n_rows; ++i) {
        for (unsigned k = i + 1; k < n_rows; ++k) {
            if (row_is_dominated(i, k)) {
                row_assignment[k] = FIX_OUT;
                ++dominated_rows;
            }
            else if (row_is_dominated(k, i)) {
                row_assignment[i] = FIX_OUT;
                ++dominated_rows;
            }
        }
    }

    std::cout << "Dominated rows: " << dominated_rows << std::endl;
}

void SetCover::fix_out_dominated_cols() {
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned j = 0; j < n_cols; ++j) {
        for (unsigned k = j + 1; k < n_cols; ++k) {
            if (col_assignment[j] != FREE || col_assignment[k] != FREE)
                continue;

            if (col_dominates(j, k)) {
                col_assignment[k] = FIX_OUT;
            }
            else if (col_dominates(k, j)) {
                col_assignment[j] = FIX_OUT;
            }
        }
        if (j % 1000 == 0) {
            std::cout << j << "\t";
        }
    }

    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time elapsed for searching dominating columns: " << time_elapsed << " s";
    std::cout << std::endl;
}

void SetCover::fix_out_heuristic_dom_cols(const std::vector<int>  & min_cost_col){
    unsigned dominated_cols = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] != FREE)
            continue;

        if (col_dom_heuristic(j, min_cost_col)) {
            col_assignment[j] = FIX_OUT;
            ++dominated_cols;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time elapsed for searching dominating columns: " << time_elapsed << " s" << std::endl;
    std::cout << "Dominated cols: " << dominated_cols << std::endl;
}


Status SetCover::get_row_status(const unsigned i) {
    return row_assignment[i];
}

Status SetCover::get_col_status(const unsigned j){
    return col_assignment[j];
}
