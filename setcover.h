#include <vector>

#ifndef SETCOVER_H
#define SETCOVER_H

struct Cell{
    unsigned row, col;
    Cell* up;
    Cell* right;
    Cell* down;
    Cell* left;
} ;

class SetCover {
private:
    unsigned int n_rows, n_cols;
    std::vector<Cell*> rows;
    std::vector<Cell*> cols;
    std::vector<unsigned> costs;
    std::vector<unsigned> row_density;
    std::vector<unsigned> col_density;

public:
    SetCover(int r, int c);

    ~SetCover();

    SetCover(const SetCover& s);

    void operator=(const SetCover& s);

    Cell* column_tail(const unsigned j);

    Cell* get_col_head(const unsigned j);

    Cell* get_row_head(const unsigned i);

    void insert_cell(const unsigned i, const unsigned j);

    void set_cost(const unsigned j, const unsigned cost);

    unsigned get_row_den(const unsigned i);

    unsigned get_col_den(const unsigned j);
};

#endif