#include <iostream>
#include <chrono>
#include "setcover.h"

unsigned SetCover::fix_essential_columns() {
    unsigned fixed_cols = 0;
    for(std::set<unsigned>::iterator i = available_row.begin(); i!=available_row.end(); ++i )
        if (row_density[*i] == 1) {
            unsigned col = get_row_head(*i)->col;
            if (col_assignment[col] == FREE) {
                col_assignment[col] = FIX_IN;
                row_assignment[*i] = FIX_OUT;
                ++fixed_cols;
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
                    if (row_density[*i] != row_density[ptr->row]) {
                        row_assignment[ptr->row] = FIX_OUT;
                    }
                    else if (*i < ptr->row) {
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

        // questa parte forse potrei farla nel pre processing 
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

void SetCover::delete_fix_out_rows(){
    for (unsigned i = 0; i < n_rows; ++i) {
        if(row_assignment[i] == FIX_OUT && rows[i] != NULL)
            remove_row(i);
    }
}

void SetCover::delete_fix_out_cols(){
    for(unsigned j=0; j< n_cols; ++j){
        if(col_assignment[j] == FIX_OUT && cols[j] != NULL)
            remove_col(j);
    }
}

unsigned SetCover::remaining_rows()
{
    return available_row.size();
}

unsigned SetCover::remaining_cols()
{
    return available_col.size();
}
