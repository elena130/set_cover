#include "setcover.h"
#include <iostream>
#include "limits.h"


SetCover::SetCover(unsigned r, unsigned c) : n_rows(r), n_cols(c), rows(r), cols(c), costs(c), row_density(r,0), col_density(c) {}

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
}

void SetCover::copy(const SetCover& s) {
    n_rows = s.n_rows;
    n_cols = s.n_cols;
    rows.resize(n_rows);
    cols.resize(n_cols);
    row_density.resize(n_rows);
    col_density.resize(n_cols);

    for (unsigned i = 0; i < n_rows; ++i) {
        row_density[i] = s.row_density[i];
        rows[i] = new Cell();
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

void SetCover::cancel_row(const unsigned i){
    Cell* prec = rows[i]->up;
    Cell* next = rows[i]->down;
    Cell* ptr = rows[i];
    Cell* delete_me;

    for (unsigned k = 0; k < row_density[i]; ++k) {
        prec->down = next;
        next->up = prec;
        delete_me = ptr;
        --col_density[ptr->col];
        ptr = ptr->right;
        delete delete_me;

        prec = ptr->up;
        next = ptr->down;
    }

    row_density[i] = 0;
    rows[i] = NULL;
    costs[i] = UINT_MAX;
    //NB: per lasciare la struttura dati coerente dovrei anche aggiornare il contatore delle righe,
    // però sto lasciando la posizione del vettore a NULL, il che non riflette esattamente il numero 
    // di righe decrementato. Non so bene come approcciarlo, perché in entrambi i casi mi sembra di rompere
    // qualcosa però si potrebbe delegare tutto questo aggiornamento alla fine, visto che potrei compattare 
    // il vettore e fare in modo che non contenga NULL alla fine. Non so se ne valga la pena 

    // visto che la cancellazione comunque viene delegata alla fine potrei semplicemente fare una grande
    // funzione che richiama questa e che poi si occupa alla fine di aggiornare il numero di righe e colonne 
}

bool SetCover::is_subset(const unsigned i, const unsigned k) {
    if (row_density[i] > row_density[k]) 
        return false;

    Cell* ptr_i = rows[i];
    Cell* ptr_k = rows[k];
    unsigned counter_i = 0;
    unsigned counter_k = 0;

    while( counter_i < row_density[i] && counter_k < row_density[k]) {
        if (ptr_i->col == ptr_k->col) {
            ptr_i = ptr_i->right;
            ptr_k = ptr_k->right;
            ++counter_i;
            ++counter_k;
        }
        else if (ptr_i->col > ptr_k->col) {
            ptr_k = ptr_k->right;
            ++counter_k;
        }
        else {
            return false;
        }
    }

    return counter_i == row_density[i];
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
