#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"
#include "setcover.h"
#include "reduction.h"

int main(int argc, char* argv[]) {
    std::fstream file;
    std::string line;
    unsigned int nr, nc;

    if (argc != 2) {
        std::cout << "Error, path to input file is not specified. Terminating" << std::endl;
        return 1;
    } 

    std::cout << "Reading matrix" << std::endl;
    Reader input(argv[1]);

    nr = input.next_int();
    nc = input.next_int();
    std::cout << nr << "x" << nc << std::endl;

    SetCover sc(nr, nc);

    for (unsigned j = 0; j < nc; ++j) {
        unsigned cost = input.next_int();
        sc.set_cost(j, cost);
        unsigned den = input.next_int();
        for (unsigned k = 0; k < den; ++k) {
            unsigned i = input.next_int();
            sc.insert_cell(i - 1, j);
        }
    }

    std::cout << "Finished building structure" << std::endl;

    Reduction reduction(sc, nr, nc);
    reduction.fix_essential_columns();
    reduction.fix_out_dominated_rows();
    reduction.fix_out_dominated_cols();
    
    for (unsigned i = 0; i < nr; i++) {
        if (reduction.get_row_status(i) == FIX_OUT) {
            sc.remove_row(i);
        }
    }

    for (unsigned j = 0; j < nc; ++j) {
        if (reduction.get_col_status(j) == FIX_OUT) {
            sc.remove_col(j);
        }
    }

    return 0;
}

