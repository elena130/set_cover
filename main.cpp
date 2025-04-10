#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"
#include "cell.h"
#include "setcover.h"

int main() {
    std::fstream file;
    std::string line;
    unsigned int nr, nc;

    std::string orig_file = "rail4872.txt";

    std::cout << "Reading matrix" << std::endl;
    Reader input(orig_file);

    nr = input.next_int();
    nc = input.next_int();
    std::cout << nr << "x" << nc << std::endl;

    SetCover sc(nr, nc);

    for (unsigned j = 0; j < nc; ++j) {
        sc.set_cost(j, input.next_int());
        sc.set_col_den(j, input.next_int());
        for (unsigned k = 0; k < sc.get_col_den(j); ++k) {
            unsigned i = input.next_int();
            sc.inc_row_den(i-1);
            sc.insert_cell(i - 1, j);
        }
    }

    std::cout << "Finished building structure" << std::endl;

    return 0;
}

