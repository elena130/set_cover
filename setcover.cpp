#include "setcover.h"
#include <iostream>
#include "limits.h"


SetCover::SetCover(unsigned r, unsigned c) : n_rows(r), n_cols(c), rows(r), cols(c), costs(c), 
row_density(r,0), col_density(c), row_assignment(r, FREE), col_assignment(c, FREE) {}

SetCover::~SetCover() {
    clear();
}

void SetCover::clear() {
    for (unsigned j = 0; j < n_cols; ++j) {
        Cell* current = cols[j];
        Cell* next;

        for (unsigned k = 0; k < col_density[j]; ++k) {
            next = current->down;
            delete current;
            current = next;
        }
    }

    n_rows = 0;
    n_cols = 0;
    rows.clear();
    cols.clear();
    costs.clear();
    row_density.clear();
    col_density.clear();
    row_assignment.clear();
    col_assignment.clear();
}

void SetCover::copy(const SetCover& s) {
    n_rows = s.n_rows;
    n_cols = s.n_cols;
    rows.resize(n_rows);
    cols.resize(n_cols);
    costs.resize(n_cols);
    row_density.resize(n_rows);
    col_density.resize(n_cols);
    row_assignment.resize(n_rows);
    col_assignment.resize(n_cols);

    for (unsigned j = 0; j < n_cols; j++) {
        costs[j] = s.costs[j];
        col_assignment[j] = s.col_assignment[j];
        Cell* ptr = s.cols[j];
        for (unsigned k = 0; k < s.col_density[j]; ++k) {
            insert_cell(ptr->row, j);
            ptr = ptr->down;
        }
    }

    for (unsigned i = 0; i < n_rows; i++) {
        row_assignment[i] = s.row_assignment[i];
    }
}

SetCover::SetCover(const SetCover& s) {
    copy(s);
}

void SetCover::operator=(const SetCover& s){
    clear();
    copy(s);
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

// check if row i is dominated by row k, i.e. the set of columns which cover i is a subset of the 
// columns which cover the row k 
bool SetCover::row_is_dominated(const unsigned i, const unsigned k) {
    if (row_density[i] > row_density[k]) 
        return false;

    if (rows[i]->up->col < rows[k]->col || rows[k]->up->col < rows[i]->col)
        return false;

    Cell* ptr_i = rows[i];
    Cell* ptr_k = rows[k];
    unsigned counter_i = 0;
    unsigned counter_k = 0;

    while( counter_i < row_density[i] && counter_k < row_density[k]) {
        if (ptr_i->col == ptr_k->col) {
            ptr_i = ptr_i->right;
            ++counter_i;
        }
        else if ( ptr_k->col > ptr_i->col){
            return false;
        }

        ptr_k = ptr_k->right;
        ++counter_k;
    }

    return counter_i == row_density[i];
}

// check if a column dominates another, i.e. when a column j covers the same rows as the 
// column k or more and the cost of j is less than the cost of k 
bool SetCover::col_dominates(const unsigned j, const unsigned k){
    if (col_density[j] < col_density[k])
        return false;

    if (costs[j] > costs[k])
        return false;

    if (cols[j]->row > cols[k]->up->row || cols[k]->row > cols[j]->up->row)
        return false;

    Cell* ptr_j = cols[j];
    Cell* ptr_k = cols[k];
    unsigned count_j = 0;
    unsigned count_k = 0;

    while (count_k < col_density[k] && count_j < col_density[j]) {
        if (ptr_j->row > ptr_k->row) {
            return false;
        } else if (ptr_k->row == ptr_j->row) {
            ptr_k = ptr_k->down;
            ++count_k;
        }
        
        ptr_j = ptr_j->down;
        ++count_j;
    }
    return count_k == col_density[k];
}

// check if a column is dominated, i.e. if there is a set of columns which cover the same rows 
// but at a minor cost 
bool SetCover::col_dom_heuristic(const unsigned j, const std::vector<int> & min_cost_col){
    unsigned sum = 0;
    Cell* ptr = cols[j];

    if (col_density[j] <= 1)
        return false;

    for (unsigned k = 0; k < col_density[j]; ++k) {
        unsigned row = ptr->row;
        sum += costs[min_cost_col[row]];
    }

    return sum <= costs[j];
}

void SetCover::remove_row(const unsigned i) {
    Cell* ptr = rows[i];
    Cell* prec_cell = rows[i]->up;
    Cell* next_cell = rows[i]->down;
    Cell* old_ptr;

    for (unsigned k = 0; k < row_density[i]; ++k) {
        unsigned j = ptr->col;
        prec_cell->down = next_cell;
        next_cell->up = prec_cell;
        --col_density[j];

        if (col_density[j] == 0) {
            cols[j] = NULL;
            costs[j] = UINT_MAX;
        } else if (ptr == get_col_head(j)) {
            cols[j] = next_cell;
        }

        old_ptr = ptr;
        ptr = ptr->right;

        prec_cell = ptr->up;
        next_cell = ptr->down;
        delete old_ptr;
    }

    rows[i] = NULL;
    row_density[i] = 0;
}

void SetCover::remove_col(const unsigned j){
    Cell* ptr = cols[j];
    Cell* prec_cell = ptr->left;
    Cell* next_cell = ptr->right;
    Cell* old_ptr;

    for (unsigned k = 0; k < col_density[j]; ++k) {
        unsigned i = ptr->row;
        prec_cell->right = next_cell;
        next_cell->left = prec_cell;

        old_ptr = ptr;
        --row_density[i];

        if (row_density[i] == 0) {
            cols[i] = NULL;
        }
        else if (ptr == get_row_head(i)) {
            rows[i] = ptr->right;
        }

        ptr = ptr->down;
        prec_cell = ptr->left;
        next_cell = ptr->right;

        delete old_ptr;
    }

    cols[j] = NULL;
    costs[j] = UINT_MAX;
    col_density[j] = 0;
}

void SetCover::set_cost(const unsigned j,  const unsigned cost) {
    costs[j] = cost;
}

unsigned SetCover::get_cost(const unsigned j) {
    return costs[j];
}

unsigned SetCover::get_row_den(const unsigned i)
{
    return row_density[i];
}

unsigned SetCover::get_col_den(const unsigned j) {
    return col_density[j];
}
