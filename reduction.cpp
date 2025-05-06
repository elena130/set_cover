#include <iostream>
#include <chrono>
#include <utility>
#include "limits.h"
#include "setcover.h"

unsigned SetCover::fix_essential_columns() {
    unsigned fixed_cols = 0;
    for(std::set<unsigned>::iterator i = available_row.begin(); i!=available_row.end(); ++i )
        if (row_density[*i] == 1) {
            unsigned col = get_row_head(*i)->col;
            if (col_assignment[col] == FREE) {
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

    return fixed_cols;
}

unsigned SetCover::fix_out_dominated_rows() {
    unsigned dominated_rows = 0;
    unsigned shortest;

    for(std::set<unsigned>::iterator i = available_row.begin(); i!=available_row.end(); ++i){
       
        shortest = rows[*i]->col;
        Cell* ptr = rows[*i];
        for (unsigned k = 0; k < col_density[*i]; ++k) {
            if (col_density[ptr->col] < col_density[shortest]) {
                shortest = ptr->col;
            }
            ptr = ptr->right;
        }

        ptr = cols[shortest];
        for (unsigned k = 0; k < col_density[shortest]; ++k) {
            if (*i != ptr->row && row_assignment[*i] == FREE && row_assignment[ptr->row] == FREE) {
                if (row_is_subset_of(*i, ptr->row)) {
                    dominated_rows++;
                    if (row_density[*i] != row_density[ptr->row] || (row_density[*i] == row_density[ptr->row] && ptr->row > *i)) {
                        row_assignment[ptr->row] = FIX_OUT;
                    }
                    else {
                        row_assignment[*i] = FIX_OUT;
                        break;
                    }
                }
            }

            ptr = ptr->down;
        }
    }
    return dominated_rows;
}


unsigned SetCover::fix_out_dominated_cols() {

    std::cout << "fixing out dominated cols without heuristic: ";

    unsigned dominated = 0;

    for(std::set<unsigned>::iterator j = available_col.begin(); j != available_col.end(); ++j){

        if (col_assignment[*j] == FIX_IN) {
            continue; 
        }

        Cell* ptr = cols[*j];
        unsigned smallest = ptr->row;

        for (unsigned k = 0; k < col_density[*j]; ++k) {
            if (row_density[ptr->row] < row_density[smallest]) {
                smallest = ptr->row;
            }
            ptr = ptr->down;
        }

        ptr = rows[smallest];
        for (unsigned k = 0; k < row_density[smallest]; ++k) {
            if (*j != ptr->col && col_assignment[*j] == FREE && col_assignment[ptr->col] == FREE) {
                if (col_is_dominated(*j, ptr->col)) {
                    ++dominated;

                    if (col_density[*j] < col_density[ptr->col] || (col_density[*j] == col_density[ptr->col] && *j > ptr->col)) {
                        col_assignment[*j] = FIX_OUT;
                        break;
                    }
                    else {
                        col_assignment[ptr->col] = FIX_OUT;
                    }
                }
            }
            ptr = ptr->right;
        }

        if (*j % 50000 == 0) {
            std::cout << *j << "\t";
        }
    }

    std::cout << std::endl;

    return dominated;
}

void SetCover::delete_fix_out_rows() {
    for (unsigned i = 0; i < n_rows; ++i) {
        if(row_assignment[i] == FIX_OUT)
            remove_row(i);
    }
}

void SetCover::delete_fix_out_cols() {
    for(unsigned j=0; j< n_cols; ++j){
        if(col_assignment[j] == FIX_OUT)
            remove_col(j);
    }
}

unsigned SetCover::remaining_rows() {
    return available_row.size();
}

unsigned SetCover::remaining_cols() {
    return available_col.size();
}

void SetCover::chvtal(const SetCover original) {

    while (!available_row.empty()) {
        float min_score = UINT_MAX;
        unsigned min_col = 0;

        for (auto j = available_col.begin(); j != available_col.end(); ++j) {
            if (col_density[*j] == 0)
                continue; 

            float score = float(costs[*j]) / float(col_density[*j]);
            if (score < min_score) {
                min_score = score;
                min_col = *j;
            }
        }

        col_assignment[min_col] = FIX_IN;
        available_col.erase(min_col);

        Cell* ptr = cols[min_col];
        for (unsigned k = 0; k < col_density[min_col]; ++k) {
            Cell* old_ptr = ptr;
            ptr = ptr->down;

            remove_row(old_ptr->row);
        }

        remove_col(min_col);
    }

    std::cout << "Solution cost before Chvatal reduction: " << solution_value(original) << std::endl;

    // last reduction 
    std::set<std::pair<unsigned, unsigned>> expensive;
    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] == FIX_IN) {
            expensive.insert(std::make_pair(costs[j], j));
        }
    }

    std::vector<std::pair<unsigned, unsigned>> ordered(expensive.begin(), expensive.end());

    for (unsigned j = 0; j < ordered.size()-1; ++j) {
        unsigned current = ordered[j].second;
        Cell* cur_ptr = original.cols[current];
        unsigned cur_index = 0;
        Cell* next_ptr = original.cols[ordered[j+1].second];
        unsigned next_index = 0;

        for (unsigned k = j + 1; k < ordered.size()-1; ++k) {
            if (cur_index == original.col_density[current] || next_index == original.col_density[next_ptr->col]) {
                break;
            }
            if (cur_ptr->row < next_ptr->row) {
                next_ptr = original.cols[ordered[k + 1].second];
                next_index = 0;
                continue;
            }
            if (cur_ptr->row == next_ptr->row) {
                cur_ptr = cur_ptr->down;
                ++cur_index;
            }

            next_ptr = next_ptr->down;
            next_index++;


        }

        if (cur_index == original.col_density[current]) {
            col_assignment[current] = FIX_OUT;
            std::cout << "ESclusa colonna " << current <<" da Chvatal soluzione" << std::endl;
        }
    }
}

bool SetCover::solution_is_correct(const SetCover original) {
    Cell* ptr;
    bool ok = true;

    for (unsigned i = 0; i < n_rows; ++i) {
        ptr = original.rows[i];
        unsigned counter = 0;

        while (counter < original.row_density[i] && col_assignment[ptr->col] != FIX_IN) {
            ++counter;
            ptr = ptr->right;
        }

        if (col_assignment[ptr->col] != FIX_IN) {
            ok = false;
            break;
        }
    }

    return ok;
}

unsigned SetCover::solution_value(const SetCover original){
    unsigned solution_cost = 0;

    for (unsigned j = 0; j < n_cols; ++j) {
        if (col_assignment[j] == FIX_IN)
            solution_cost += original.costs[j];
    }

    return solution_cost;
}
