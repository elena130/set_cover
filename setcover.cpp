#include "setcover.h"
#include <iostream>
#include "limits.h"


SetCover::SetCover(unsigned r, unsigned c) : n_rows(r), n_cols(c), rows(r), cols(c), costs(c), 
row_density(r,0), col_density(c), row_assignment(r, FREE), col_assignment(c, FREE), min_cost_col(r, -1){
    for (unsigned i = 0; i < r; ++i) {
        available_row.insert(available_row.end(), i);
    }
    for (unsigned j = 0; j < c; ++j) {
        available_col.insert(available_col.end(), j);
    }
}

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
    min_cost_col.clear();
    available_row.clear();
    available_col.clear();
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
    min_cost_col.resize(n_rows);
    available_row = s.available_row;
    available_col = s.available_col;

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
        min_cost_col[i] = s.min_cost_col[i];
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

    if (min_cost_col[i] == -1 || costs[min_cost_col[i]] > costs[j]) {
        min_cost_col[i] = j;
    }

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
bool SetCover::row_is_subset_of(const unsigned i, const unsigned k) {
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

// check if a column j is dominated by column k, i.e. when a column j covers a subset of the 
// rows covered by k 
bool SetCover::col_is_dominated(const unsigned j, const unsigned k){
    if (col_density[j] > col_density[k])
        return false;

    if (costs[j] < costs[k])
        return false;

    Cell* ptr_j = cols[j];
    Cell* ptr_k = cols[k];
    unsigned count_j = 0;
    unsigned count_k = 0;

    while (count_k < col_density[k] && count_j < col_density[j]) {
        if (ptr_k->row > ptr_j->row) {
            return false;
        }
        else if (col_density[k] - count_k < col_density[j] - count_j) {
            return false;
        } else if (ptr_k->row == ptr_j->row) {
            ptr_j = ptr_j->down;
            ++count_j;
        }
        ptr_k = ptr_k->down;
        ++count_k;
    }
    return count_j == col_density[j];
}

// check if a column is dominated, i.e. if there is a set of columns which cover the same rows 
// but at a minor cost 
bool SetCover::is_col_dominated_heuristic(const unsigned j){
    unsigned sum = 0;
    std::set<unsigned> added;
    std::set<unsigned> not_covered;

    Cell* ptr = cols[j];

    for (unsigned k = 0; k < col_density[j]; ++k) {
        not_covered.insert(ptr->row);
        ptr = ptr->down;
    }

    ptr = cols[j];
    Cell* to_add;

    for (unsigned k = 0; k < col_density[j]; ++k) {
        unsigned row = ptr->row;
        if (col_assignment[min_cost_col[row]] != FIX_OUT && added.find(min_cost_col[row]) == added.end()) {
            sum += costs[min_cost_col[row]];

            if (sum > costs[j]) {
                return false;
            }

            added.insert(min_cost_col[row]);
            to_add = cols[min_cost_col[row]];
            for (unsigned h = 0; h < col_density[min_cost_col[row]]; ++h) {
                not_covered.erase(to_add->row);
                to_add = to_add->down;
            }
        }
        
        if (not_covered.empty()) {
            return true;
        }
        ptr = ptr->down;
    }

    return sum <= costs[j];
}

// removes a row from the set cover, if present. Otherwise it doesn't modify the set cover. 
void SetCover::remove_row(const unsigned i) {
    if (rows[i] == NULL)
        return;

    Cell* ptr = rows[i];
    Cell* prec_cell = rows[i]->up;
    Cell* succ_cell = rows[i]->down;
    Cell* old_ptr;

    // first thing: fix the pointers to deattach the row from the matrix 
    for (unsigned k = 0; k < row_density[i]; ++k) {
        prec_cell->down = succ_cell;
        succ_cell->up = prec_cell;

        if (ptr == get_col_head(ptr->col))
            cols[ptr->col] = ptr->down;

        ptr = ptr->right;
        prec_cell = ptr->up;
        succ_cell = ptr->down;
    }

    // second thing: delete the row and update the metdata to leave the structure consistent 
    ptr = rows[i];
    for (unsigned k = 0; k < row_density[i]; ++k) {
        old_ptr = ptr;
        unsigned j = ptr->col;
        ptr = ptr->right;
        delete old_ptr;

        --col_density[j];
        if (col_density[j] == 0) {
            cols[j] = NULL;
            costs[j] = UINT_MAX;
            available_col.erase(j);
        }
    }

    rows[i] = NULL;
    row_density[i] = 0;
    available_row.erase(i);
}

// removes col j from the set cover problem, if present. Otherwise it leaves the set cover
// unchanged. 
void SetCover::remove_col(const unsigned j){
    if (cols[j] == NULL)
        return;

    Cell* ptr = cols[j];
    Cell* prec_cell = cols[j]->left;
    Cell* succ_cell = cols[j]->right;

    // fixing the pointers to deattach the column from the matrix 
    for (unsigned k = 0; k < col_density[j]; ++k) {
        prec_cell->right = succ_cell;
        succ_cell->left = prec_cell;

        if (ptr == get_row_head(ptr->row)) {
            rows[ptr->row] = ptr->right;
        }

        ptr = ptr->down;
        prec_cell = ptr->left;
        succ_cell = ptr->right;
    }

    Cell* old_ptr;
    ptr = cols[j];

    for (unsigned k = 0; k < col_density[j]; ++k) {
        old_ptr = ptr;
        unsigned i = ptr->row;
        ptr = ptr->down;

        --row_density[i];
        if (row_density[i] == 0) {
            rows[i] = NULL;
            available_row.erase(i);
        }

        delete old_ptr;
    }

    cols[j] = NULL;
    col_density[j] = 0;
    available_col.erase(j);
    costs[j] = UINT_MAX;
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
