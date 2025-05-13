#include <iostream>
#include <chrono>
#include <utility>
#include "limits.h"
#include "setcover.h"

unsigned SetCover::fix_essential_columns() {
    unsigned fixed_cols = 0;
    for(std::set<unsigned>::iterator i = uncovered_rows.begin(); i!=uncovered_rows.end(); ++i )
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

    for(std::set<unsigned>::iterator i = uncovered_rows.begin(); i!=uncovered_rows.end(); ++i){
       
        // find the shortest column which covers the row
        shortest = rows[*i]->col;
        Cell* ptr = rows[*i];
        for (unsigned k = 0; k < row_density[*i]; ++k) {
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

    std::cout << "Fixing out dominated cols counter: ";

    unsigned dominated = 0;

    for(std::set<unsigned>::iterator j = available_col.begin(); j != available_col.end(); ++j){

        if (col_assignment[*j] == FIX_IN) {
            continue; 
        }

        // find the shortes row that is covered by the column
        Cell* ptr = cols[*j];
        unsigned shortest = ptr->row;

        for (unsigned k = 0; k < col_density[*j]; ++k) {
            if (row_density[ptr->row] < row_density[shortest]) {
                shortest = ptr->row;
            }
            ptr = ptr->down;
        }

        ptr = rows[shortest];
        for (unsigned k = 0; k < row_density[shortest]; ++k) {
            if (*j != ptr->col && col_assignment[*j] == FREE && col_assignment[ptr->col] == FREE) {
                if (col_is_dominated(*j, ptr->col)) {
                    ++dominated;

                    if (col_density[*j] < col_density[ptr->col] || (col_density[*j] == col_density[ptr->col] && *j > ptr->col)) {
                        col_assignment[*j] = FIX_OUT;
                        //std::cout << "Colonna " << *j << " dominata da " << ptr->col << std::endl;
                        break;
                    }
                    else {
                        col_assignment[ptr->col] = FIX_OUT;
                        //std::cout << "Colonna " << ptr->col << " dominata da " << *j << std::endl;
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

unsigned SetCover::fix_out_dominated_cols_set() {

    std::cout << "Fixing out cols dominated by a group of cols. Counter: ";

    unsigned dominated = 0;

    for (std::set<unsigned>::iterator j = available_col.begin(); j != available_col.end(); ++j) {

        if (col_assignment[*j] != FREE) {
            continue;
        }

        if (column_is_set_dominated(*j, col_assignment)) {
            col_assignment[*j] = FIX_OUT;
            std::cout << std::endl << "SET R" << * j << " is dominated by a set" << std::endl;
            ++dominated;
        }
    }

    std::cout << std::endl;

    return dominated;
}

bool SetCover::column_is_set_dominated(const unsigned j, const std::vector<Status> assegnamento) {
    unsigned set_cost = 0;
    bool added = false;
    std::set<unsigned> covered_rows;

    for (unsigned k = 0; k < n_cols; ++k) {
        if (assegnamento[k] != FIX_IN)
            continue;

        if (k == j)
            continue;

        Cell* j_cell = cols[j];
        unsigned j_counter = 0;
        Cell* other_cel = cols[k];
        unsigned other_counter = 0;
        added = false;

        while (other_counter != col_density[k] && j_counter != col_density[j]) {
            
            if (j_cell->row < other_cel->row) {
                j_cell = j_cell->down;
                ++j_counter;
            } else if (j_cell->row == other_cel->row) {
                j_cell = j_cell->down;
                ++j_counter;
                if (!added) {
                    set_cost += costs[other_cel->col];
                }

                covered_rows.insert(j_cell->row);
                added = true;
                other_cel = other_cel->down;
                ++other_counter;

            }else if (j_cell->row > other_cel->row) {
                other_cel = other_cel->down;
                ++other_counter;
            }
        }

        if (covered_rows.size() == col_density[j]) {
            break;
        }
    }

    if (set_cost > costs[j] || covered_rows.size() != col_density[j])
        return false;
    else if (covered_rows.size() == col_density[j])
        return true;
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
    return uncovered_rows.size();
}

unsigned SetCover::remaining_cols() {
    return available_col.size();
}