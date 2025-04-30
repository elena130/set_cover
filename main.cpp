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

    SetCover original_sc(nr, nc);

    for (unsigned j = 0; j < nc; ++j) {
        original_sc.set_cost(j, input.next_int());
        unsigned den = input.next_int();
        for (unsigned k = 0; k < den; ++k) {
            unsigned i = input.next_int();
            original_sc.insert_cell(i - 1, j);
        }
    }

    std::cout << "Finished building structure" << std::endl;

    // mettere tutto in un while e ripeti il tutto fino a quando non fai più riduzioni 
    // prova per prima cosa a mettere le cancellazioni dopo che fai i controlli per tutte e tre 
    SetCover sc(original_sc);
    sc.fix_essential_columns();
    std::cout << "Righe dominate: " << sc.fix_out_dominated_rows() << std::endl;
    std::cout << "Colonne dominate: " << sc.fix_out_dominated_cols() << std::endl;
    
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

