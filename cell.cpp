#include "cell.h"


Cell::Cell() : row(), col(), up(), right(), down(), left() {}

Cell::Cell(const unsigned r, const unsigned c) : row(r), col(c), up(), right(), down(), left() {}

Cell::~Cell() {
   
}

Cell::Cell(const Cell& c) : row(c.row), col(c.col) {
    up = c.up;
    right = c.right;
    down = c.down;
    left = c.left;
}

void Cell::operator=(const Cell& c){
    row = c.row;
    col = c.col;
    up = c.up;
    right = c.right;
    down = c.down;
    left = c.left;
}

Cell* Cell::get_up() {
    return up;
}

Cell* Cell::get_rigth() {
    return right;
}

Cell* Cell::get_down() {
    return down;
}

Cell* Cell::get_left() {
    return left;
}

void Cell::set_up(Cell* c) {
    up = c;
}

void Cell::set_right(Cell* c) {
    right = c;
}

void Cell::set_down(Cell* c) {
    down = c;
}

void Cell::set_left(Cell* c) {
    left = c;
}