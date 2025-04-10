#include "setcover.h"
#include <iostream>

SetCover::SetCover(int r, int c) : n_rows(r), n_cols(c), rows(r), cols(c), costs(c), row_density(r,0), col_density(c) {}

SetCover::~SetCover() {

    for (unsigned j = 0; j < n_cols; ++j) {
        Cell* current = cols[j];
        Cell* next;

        for(unsigned k =0; k<col_density[j]; ++k){
            next = current->get_down();
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
        rows[i] = new Cell(s.rows[i]->get_row(), s.rows[i]->get_col());
    }

    for (unsigned j = 0; j < n_cols; j++) {
        col_density[j] = s.col_density[j];
        Cell* ptr = s.cols[j];
        for (unsigned k = 0; k < col_density[j]; ++k) {
            insert_cell(ptr->get_row(), j);
            ptr = ptr->get_down();
        }
    }
}

void SetCover::operator=(const SetCover& s){
}

void SetCover::insert_cell(const unsigned i, const unsigned j) {
    Cell* c = new Cell(i,j);
    Cell* tail = cols[j] == NULL ? NULL : cols[j]->get_upper();
    
    if (tail == NULL) {
        cols[j] = c;
        cols[j]->set_down(cols[j]);
        cols[j]->set_upper(cols[j]);
    }
    else {
        tail->set_down(c);
        c->set_upper(tail);
        c->set_down(cols[j]);
        cols[j]->set_upper(c);
    }

    if (rows[i] == NULL) {
        rows[i] = c;
        c->set_right(c);
        c->set_left(c);
    }
    else {
        Cell* prec = rows[i]->get_left();
        prec->set_right(c);
        c->set_left(prec);
        c->set_right(rows[i]);
        rows[i]->set_left(c);
    }
    
}

void SetCover::set_cost(const unsigned j,  const unsigned cost) {
    costs[j] = cost;
}

void SetCover::inc_row_den(const unsigned i) {
    row_density[i]++;
}

void SetCover::set_col_den(const unsigned j, const unsigned den) {
    col_density[j] = den;
}

unsigned SetCover::get_col_den(const unsigned j) {
    return col_density[j];
}
