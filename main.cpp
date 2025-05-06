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

    SetCover reduced_sc(original_sc);

    unsigned deletion;
    unsigned deleted_cols = 0;
    unsigned deleted_rows = 0;

    do {
        deletion = 0;
        unsigned d_cols = reduced_sc.fix_essential_columns();
        unsigned d_rows = reduced_sc.fix_out_dominated_rows();
        d_cols += reduced_sc.fix_out_dominated_cols();

        deletion += d_rows;
        deletion += d_cols;
        deleted_cols += d_cols;
        deleted_rows += d_rows;

        reduced_sc.delete_fix_out_rows();
        reduced_sc.delete_fix_out_cols();
        std::cout << "Remaining rows: " << reduced_sc.remaining_rows() << std::endl << "Remaining cols: " << reduced_sc.remaining_cols() << std::endl;


    } while (deletion != 0);

    reduced_sc.chvtal(original_sc);

    if (reduced_sc.solution_is_correct(original_sc)) 
        std::cout << "Soluzione corretta";
    else 
        std::cout << "soluzione errata";

    std::cout << "Solution cost: " << reduced_sc.solution_value(original_sc) << std::endl;
    
    return 0;
}

