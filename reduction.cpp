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
       
        // find the shortest column which covers the row
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

unsigned SetCover::fix_out_dominated_cols_set() {

    std::cout << "Fixing out cols dominated by a group of cols. Counter: ";

    unsigned dominated = 0;

    for (std::set<unsigned>::iterator j = available_col.begin(); j != available_col.end(); ++j) {

        if (col_assignment[*j] != FREE) {
            continue;
        }

        // find the shortes row that is covered by the column
        Cell* ptr = cols[*j];
        unsigned short_row = ptr->row;

        for (unsigned k = 0; k < col_density[*j]; ++k) {
            if (row_density[ptr->row] < row_density[short_row]) {
                short_row = ptr->row;
            }
            ptr = ptr->down;
        }

        // puntatore alla colonna di cui devo controllare se è dominata
        Cell* j_ptr = cols[*j];
        unsigned j_counter = 0;
        // puntatore alla riga più corta coperta da j
        Cell* short_row_ptr = rows[short_row];
        unsigned shortest_counter = 0;

        bool added = false;
        // quante colonne sono necessarie per coprire j
        unsigned set_cost = 0;

        // scorro la riga più corta
        for (unsigned k = 0; k < row_density[short_row]; ++k) {
            if (*j != short_row_ptr->col && col_assignment[short_row_ptr->col] != FIX_OUT) {
                Cell* ptr = cols[short_row_ptr->col];
                unsigned counter = 0;

                while (j_counter != col_density[*j] && shortest_counter != col_density[short_row] ) {
                   
                    if (j_ptr->row < ptr->row || counter == col_density[ptr->col]) {
                        short_row_ptr = short_row_ptr->right;
                        ptr = cols[short_row_ptr->col];
                        counter = 0;
                        added = false;
                        continue;
                    }
                    if (j_ptr->row == ptr->row) {
                        j_ptr = j_ptr->down;
                        ++j_counter;
                        if (!added) {
                            set_cost+= costs[ptr->col];
                        }
                        added = true;
                    }

                    if (set_cost > costs[*j]) {
                        break;
                    }

                    ptr = ptr->down;
                    counter++;
                }
            }
            else {
                short_row_ptr = short_row_ptr->right;
            }

            
        }

        if (j_counter == col_density[*j] && set_cost <= costs[*j]) {
            col_assignment[*j] = FIX_OUT;
            dominated++;
            continue;
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