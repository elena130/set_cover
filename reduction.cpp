#include <iostream>
#include <chrono>
#include "limits.h"
#include "setcover.h"

unsigned SetCover::fix_essential_columns(const bool first_red, std::vector<bool> modified_rows) {
    unsigned fixed_cols = 0;
    for(unsigned i : available_row )
        if (row_density[i] == 1) {
            unsigned col = get_row_head(i)->col;
            if (col_assignment[col] == FREE) {
                if (first_red || modified_rows[i]) {
                    col_assignment[col] = FIX_IN;
                    ++fixed_cols;
                
                    // fix out all the rows which are covered by the fixed column 
                    Cell* ptr = cols[col];
                    for (unsigned k = 0; k < col_density[col]; ++k) {
                        row_assignment[ptr->row] = FIX_OUT;
                        ptr = ptr->down;
                    }
                }
            }
        }

    return fixed_cols;
}

unsigned SetCover::fix_out_dominated_rows(const bool first_red, const std::vector<bool> & modified_rows) {
    unsigned dominated_rows = 0;
    unsigned shortest;

    for(unsigned i : available_row ){
        shortest = rows[i]->col;
        Cell* ptr = rows[i];
        for (unsigned k = 0; k < row_density[i]; ++k) {
            if (col_density[ptr->col] < col_density[shortest]) {
                shortest = ptr->col;
            }
            ptr = ptr->right;
        }

        ptr = cols[shortest];
        for (unsigned k = 0; k < col_density[shortest]; ++k) {
            if (i != ptr->row && row_assignment[i] == FREE && row_assignment[ptr->row] == FREE) {
                if (first_red || modified_rows[i]) {
                    if (row_is_subset_of(i, ptr->row)) {
                        dominated_rows++;
                        if (row_density[i] != row_density[ptr->row] || ptr->row > i) {
                            row_assignment[ptr->row] = FIX_OUT;
                        }
                        else {
                            row_assignment[i] = FIX_OUT;
                            break;
                        }
                    }
                }
            }
            ptr = ptr->down;
        }
    }
    return dominated_rows;
}


unsigned SetCover::fix_out_dominated_cols(const bool first_red, std::vector<bool> modified) {

    std::cout << "Fixing out dominated cols counter: ";

    unsigned dominated = 0;

    for(unsigned j : available_col){

        if (col_assignment[j] == FIX_IN) {
            continue; 
        }

        Cell* ptr = cols[j];
        unsigned smallest = ptr->row;

        for (unsigned k = 0; k < col_density[j]; ++k) {
            if (row_density[ptr->row] < row_density[smallest]) {
                smallest = ptr->row;
            }
            ptr = ptr->down;
        }

        ptr = rows[smallest];
        for (unsigned k = 0; k < row_density[smallest]; ++k) {
            if (j != ptr->col && col_assignment[j] == FREE && col_assignment[ptr->col] == FREE) {
                if (first_red || modified[j]) {
                    if (col_is_dominated(j, ptr->col)) {
                        ++dominated;

                        if (col_density[j] != col_density[ptr->col] || col_density[j] < col_density[ptr->col] ) {
                            col_assignment[j] = FIX_OUT;
                            break;
                        }
                        else {
                            col_assignment[ptr->col] = FIX_OUT;
                        }
                    }
                }

            }
            ptr = ptr->right;
        }

        if (j % 50000 == 0) {
            std::cout << j << "\t";
        }
    }

    std::cout << std::endl;

    return dominated;
}

unsigned SetCover::fix_out_cols_dom_set(const bool first_red, const std::vector<bool>& modified) {
    unsigned default_value = n_cols + 1;
    std::vector<unsigned> min_cost_col(n_rows, default_value);
    std::set<unsigned> added;
    unsigned cost;
    unsigned fixed_out = 0;

    // for each row find out the minimum cost columns which covers it
    for (unsigned i = 0; i < n_rows; i++) {
        Cell* cell = rows[i];
        unsigned min = UINT_MAX;
        for (unsigned k = 0; k < row_density[i]; ++k) {
            if (min_cost_col[i] == default_value || costs[cell->col] < min) {
                min_cost_col[i] = cell->col;
                min = costs[cell->col];
            }
            cell = cell->right;
        }
    }
    
    // consider ordering the available columns by their cost and iterate by decreasing cost
    for (auto j = available_col.crbegin(); j != available_col.crend(); ++j) {
        unsigned idx = *j;
        if (!first_red && !modified[idx]) {
            continue;
        }

        Cell* c = cols[idx];
        added.clear(); 
        cost = 0;
        bool is_covered = true;

        c = cols[idx];
        for (unsigned k = 0; k < col_density[idx]; ++k) {
            if (min_cost_col[c->row] == idx) {
                is_covered = false;
                break;
            }
            if (added.find(min_cost_col[c->row]) == added.end()) {
                cost += costs[min_cost_col[c->row]];
                if (cost > costs[idx]) {
                    break;
                }
            }
            added.insert(min_cost_col[c->row]);
            c = c->down;
        }

        if (costs[idx] >= cost && is_covered ) {
            col_assignment[idx] = FIX_OUT;
            fixed_out++;
        }
    }

    return fixed_out;
}

void SetCover::delete_fix_out_rows(std::vector<bool>& modified_cols) {
    for (unsigned i = 0; i < n_rows; ++i) {
        if(row_assignment[i] == FIX_OUT)
            remove_row(i, modified_cols);
    }
}

void SetCover::delete_fix_out_cols(std::vector<bool>& modified_rows) {
    for(unsigned j=0; j< n_cols; ++j){
        if(col_assignment[j] == FIX_OUT)
            remove_col(j, modified_rows);
    }
}

unsigned SetCover::remaining_rows() {
    return available_row.size();
}

unsigned SetCover::remaining_cols() {
    return available_col.size();
}
