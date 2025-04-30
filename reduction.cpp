#include <iostream>
#include <chrono>
#include "setcover.h"

unsigned SetCover::fix_essential_columns() {
    unsigned fixed_cols = 0;
    for (unsigned i = 0; i < n_rows; ++i) {
        if (row_density[i] == 1) {
            col_assignment[get_row_head(i)->col] = FIX_IN;
            ++fixed_cols;
            row_assignment[i] = FIX_OUT;
        }
    }

    return fixed_cols;
}

/*
unsigned SetCover::fix_out_dominated_rows() {
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
    return dominated_rows;
}
*/

unsigned SetCover::fix_out_dominated_rows() {
    unsigned dominated_rows = 0;
    unsigned short_col;

    for (unsigned i = 0; i < n_rows; ++i) {
       
        short_col = rows[i]->col;
        Cell* ptr = rows[i];
        for (unsigned k = 0; k < col_density[i]; ++k) {
            if (col_density[ptr->col] < col_density[short_col]) {
                short_col = ptr->col;
            }
            ptr = ptr->right;
        }

        ptr = cols[short_col];
        for (unsigned k = 0; k < col_density[short_col]; ++k) {
            if (row_assignment[i] != FREE || row_assignment[ptr->row] != FREE)
                continue; 

            if (row_is_dominated(i, ptr->row)) {
                if (i > ptr->row)  row_assignment[i] = FIX_OUT; else row_assignment[ptr->row] = FIX_OUT;
                ++dominated_rows;
            }
        }
    }
    return dominated_rows;
}


unsigned SetCover::fix_out_dominated_cols() {
    auto start = std::chrono::high_resolution_clock::now();
    unsigned dominated = 0;

    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] != FREE)
            continue;

        Cell* ptr = cols[j];
        unsigned index = ptr->row;

        // questa parte forse potrei farla nel pre processing 
        for (unsigned k = 0; k < col_density[j]; ++k) {
            if (row_density[ptr->row] < row_density[index]) {
                index = ptr->row;
            }
            ptr = ptr->down;
        }

        ptr = rows[index];
        for (unsigned k = 0; k < row_density[index]; ++k) {
            if (col_assignment[k] != FREE || col_assignment[ptr->col] != FREE) {
                continue;
            }

            if (j != ptr->col && col_is_dominated(j, ptr->col)) {
                if (j < ptr->col) {
                    col_assignment[ptr->col] = FIX_OUT;
                }
                else {
                    col_assignment[j] = FIX_OUT;
                }
                ++dominated;
            }
           
            ptr = ptr->right;
        }
    }

    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time elapsed for searching dominating columns: " << time_elapsed << " s";
    std::cout << std::endl;
    return dominated;
}

unsigned SetCover::fix_out_heuristic_dom_cols(){
    unsigned dominated_cols = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] != FREE)
            continue;

        if (col_dom_heuristic(j)) {
            col_assignment[j] = FIX_OUT;
            ++dominated_cols;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time elapsed for searching dominating columns: " << time_elapsed << " s" << std::endl;
    std::cout << "Dominated cols: " << dominated_cols << std::endl;
    return dominated_cols;
}


Status SetCover::get_row_status(const unsigned i) {
    return row_assignment[i];
}

Status SetCover::get_col_status(const unsigned j){
    return col_assignment[j];
}
