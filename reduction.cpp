#include "reduction.h"

void Reduction::copy(const Reduction& r) {
    n_rows = r.n_rows;
    n_cols = r.n_cols;
    rows.resize(r.n_rows, FREE);
    columns.resize(r.n_cols, FREE);

    for (unsigned i = 0; i < n_rows; ++i) {
        rows[i] = r.rows[i];
    }

    for (unsigned j = 0; j < n_cols; ++j) {
        columns[j] = r.columns[j];
    }
}


Reduction::Reduction(SetCover& s, const unsigned nr, const unsigned nc) : n_rows(nr), n_cols(nc), sc(s), rows(nr, FREE), columns(nc, FREE) {
}

Reduction::~Reduction() {
}

Reduction::Reduction(const Reduction& r) : sc(r.sc) {
    copy(r);
}

void Reduction::operator=(const Reduction& r) {
    sc = r.sc;
    copy(r);
}

void Reduction::fix_essential_columns() {
    unsigned fixed_cols = 0;
    for (unsigned i = 0; i < n_rows; ++i) {
        if (sc.get_row_den(i) == 1) {
            columns[sc.get_row_head(i)->col] = FIX_IN;
            ++fixed_cols;
        }
    }

    std::cout << "Fixed columns: " << fixed_cols << std::endl;
}

void Reduction::fix_out_dominated_rows() {
    unsigned dominated_rows = 0;

    for (unsigned i = 0; i < n_rows; ++i) {
        for (unsigned k = i + 1; k < n_rows; ++k) {
            if (sc.row_is_dominated(i, k)) {
                rows[k] = FIX_OUT;
                ++dominated_rows;
            }
            else if (sc.row_is_dominated(k, i)) {
                rows[i] = FIX_OUT;
                ++dominated_rows;
            }
        }
    }

    std::cout << "Dominated rows: " << dominated_rows << std::endl;
}

void Reduction::fix_out_dominated_cols() {
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned j = 0; j < n_cols; ++j) {
        for (unsigned k = j + 1; k < n_cols; ++k) {
            if (columns[j] != FREE || columns[k] != FREE)
                continue;

            if (sc.col_dominates(j, k)) {
                columns[k] = FIX_OUT;
            }
            else if (sc.col_dominates(k, j)) {
                columns[j] = FIX_OUT;
            }
        }
        if (j % 1000 == 0) {
            std::cout << j << "\t";
        }
    }

    std::cout << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time elapsed for searching dominating columns: " << time_elapsed << " s";
    std::cout << std::endl;
}

void Reduction::fix_out_heuristic_dom_cols(const std::vector<int>  & min_cost_col){
    unsigned dominated_cols = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned j = 0; j < n_cols; ++j) {
        if (columns[j] != FREE)
            continue;

        if (sc.col_dom_heuristic(j, min_cost_col)) {
            columns[j] = FIX_OUT;
            ++dominated_cols;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Time elapsed for searching dominating columns: " << time_elapsed << " s" << std::endl;
    std::cout << "Dominated cols: " << dominated_cols << std::endl;
}


Status Reduction::get_row_status(const unsigned i) {
    return rows[i];
}

Status Reduction::get_col_status(const unsigned j){
    return columns[j];
}
