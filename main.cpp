#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include "parser.h"
#include "setcover.h"

int main(int argc, char* argv[]) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::fstream file;
    std::string line;
    unsigned int nr, nc;

    if (argc != 2) {
        std::cout << "Error, path to input file is not specified. Terminating" << std::endl;
        return 1;
    } 

    std::cout << "READING MATRIX" << std::endl;
    Reader input(argv[1]);

    nr = input.next_int();
    nc = input.next_int();
    std::cout << nr << "x" << nc << std::endl;

    SetCover original_sc(nr, nc);

    // loading structure from rail.txt files
    if (input.get_file_name().find("rail") != std::string::npos ) {
        for (unsigned j = 0; j < nc; ++j) {
            unsigned costo = input.next_int();
            original_sc.set_cost(j, costo);
            unsigned den = input.next_int();

            for (unsigned k = 0; k < den; ++k) {
                unsigned i = input.next_int();
                original_sc.insert_cell(i - 1, j);
            }
        }
    }
    // building structure from any other file 
    else {
        for (unsigned j = 0; j < nc; ++j) {
            original_sc.set_cost(j, input.next_int());
        }

        for (unsigned i = 0; i < nr; ++i) {
            unsigned den = input.next_int();
            for (unsigned k = 0; k < den; k++) {
                unsigned j = input.next_int();
                original_sc.insert_cell(i, j - 1);
            }
        }
    }

    std::cout << "Finished building structure" << std::endl << std::endl;
    std::cout << "REDUCTIONS" << std::endl;

    SetCover sc(original_sc);
    std::vector<bool> next_modified_cols(nc, false);
    std::vector<bool> prec_modified_cols;
    unsigned deleted;
    bool first_reduction = true;

    do {
        deleted = 0;
        deleted = sc.fix_essential_columns();
        deleted = sc.fix_out_dominated_rows();
        deleted += sc.fix_out_dominated_cols(first_reduction, next_modified_cols);
        deleted += sc.fix_out_cols_dom_set(first_reduction, next_modified_cols);

        //std::fill(next_modified_cols.begin(), next_modified_cols.end(), false);
        next_modified_cols = std::vector<bool>(nc, false);
        sc.delete_fix_out_rows(next_modified_cols);
        sc.delete_fix_out_cols();
        first_reduction = false;
        std::cout << "Remaining rows: " << sc.remaining_rows() << std::endl << "Remaining cols: " << sc.remaining_cols() << std::endl;

    } while (deleted != 0);

    std::cout << std::endl;
    std::cout << "CHVATAL" << std::endl;
    std::set<unsigned> selected;
    sc.chvtal();

    if (sc.solution_is_correct(original_sc)) 
        std::cout << "Solution is correct";
    else 
        std::cout << "Solution is wrong, check your code";

    std::cout << std::endl;

    unsigned sol_val = sc.solution_value(original_sc);
    std::cout << "Solution cost: " << sol_val << std::endl;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    std::cout << "Time difference = " << time << "[s]" << std::endl;
    
    std::cout << std::endl;
    std::cout << nr << "\t" << nc << "\t";
    std::cout << sc.remaining_rows() << "\t" << sc.remaining_cols() << "\t" << sol_val << "\t" << 0 << "\t" << time;

    return 0;
}

