#include "setcover.h"
#include <iostream>

SetCover::SetCover(unsigned r, unsigned c) : n_rows(r), n_cols(c), rows(r), cols(c), costs(c), row_density(r,0), col_density(c) {}

SetCover::~SetCover() {

    for (unsigned j = 0; j < n_cols; ++j) {
        Cell* current = cols[j];
        Cell* next;

        for(unsigned k =0; k<col_density[j]; ++k){
            next = current->down;
            delete current;
            current = next;
        }
    }
}

SetCover::SetCover(const SetCover& s) : n_rows(s.n_rows), n_cols(s.n_cols) {
    rows.resize(n_rows);
    cols.resize(n_cols);
    row_density.resize(n_rows);
    col_density.resize(n_cols);

    for (unsigned i = 0; i < n_rows; ++i) {
        row_density[i] = s.row_density[i];
        rows[i] =  new Cell();
        rows[i]->row = s.rows[i]->row;
        rows[i]->col = s.rows[i]->col;
    }

    for (unsigned j = 0; j < n_cols; j++) {
        col_density[j] = s.col_density[j];
        Cell* ptr = s.cols[j];
        for (unsigned k = 0; k < col_density[j]; ++k) {
            insert_cell(ptr->row, j);
            ptr = ptr->down;
        }
    }
}

void SetCover::operator=(const SetCover& s){
    *this = SetCover(s);
}

Cell* SetCover::column_tail(const unsigned j) {
    if (cols[j] == NULL)
       return NULL;
    else 
       return cols[j]->up;
}

Cell* SetCover::get_col_head(const unsigned j)
{
    return cols[j];
}

Cell* SetCover::get_row_head(const unsigned i)
{
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

    // setting right and left pointer 
    if (rows[i] == NULL) {
        rows[i] = c;
        c->right = c;
        c->left = c;
    }
    else {
        Cell* prec = rows[i]->left;
        prec->right = c;
        c->left = prec;
        c->right = rows[i];
        rows[i]->left = c;
    }
    
}

void SetCover::set_cost(const unsigned j,  const unsigned cost) {
    costs[j] = cost;
}

unsigned SetCover::get_row_den(const unsigned i)
{
    return row_density[i];
}

unsigned SetCover::get_col_den(const unsigned j) {
    return col_density[j];
}
