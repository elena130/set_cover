#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"
#include "setcover.h"

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
    // indice della colonna di minor costo che copre la riga 
    std::vector<int> min_cost_col(nr, -1);

    for (unsigned j = 0; j < nc; ++j) {
        unsigned cost = input.next_int();
        sc.set_cost(j, cost);
        unsigned den = input.next_int();
        for (unsigned k = 0; k < den; ++k) {
            unsigned i = input.next_int();
            sc.insert_cell(i - 1, j);

            if (min_cost_col[i-1] == -1 || sc.get_cost(min_cost_col[i-1]) > cost ) {
                min_cost_col[i-1] = j;
            }
        }
    }

    std::cout << "Finished building structure" << std::endl;

    sc.fix_essential_columns();
    sc.fix_out_dominated_rows();
    sc.fix_out_heuristic_dom_cols(min_cost_col);
    
    for (unsigned i = 0; i < nr; i++) {
        if (sc.get_row_status(i) == FIX_OUT) {
            sc.remove_row(i);
        }
    }

    for (unsigned j = 0; j < nc; ++j) {
        if (sc.get_col_status(j) == FIX_OUT) {
            sc.remove_col(j);
        }
    }

    return 0;
}

