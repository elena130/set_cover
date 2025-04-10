#ifndef CELL_H
#define CELL_H

class Cell {
private:
    unsigned row, col;
    Cell* up;
    Cell* right;
    Cell* down;
    Cell* left;

public:
    Cell();

    Cell(const unsigned r, const unsigned c);

    ~Cell();

    Cell(const Cell& c);

    void operator=(const Cell& c);

    Cell* get_up();

    Cell* get_rigth();

    Cell* get_down();

    Cell* get_left();

    void set_up(Cell* c);

    void set_right(Cell* c);

    void set_down(Cell* c);

    void set_left(Cell* c);


};

#endif 