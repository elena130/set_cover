#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"
#include "setcover.h"

int main(int argc, char* argv[]) {
    std::fstream file;
    std::string line;
    unsigned int nr, nc;
    Assignment assignment;

    if (argc != 2) {
        std::cout << "Error, path to input file is not specified. Terminating" << std::endl;
        return 1;
    } 

    std::cout << "READING matrix" << std::endl;
    Reader input(argv[1]);

    nr = input.next_int();
    nc = input.next_int();
    std::cout << nr << "x" << nc << std::endl;

    SetCover original_sc(nr, nc);
    assignment.rows.resize(nr, FREE);
    assignment.cols.resize(nc, FREE);

    for (unsigned j = 0; j < nc; ++j) {
        original_sc.set_cost(j, input.next_int());
        unsigned den = input.next_int();
        for (unsigned k = 0; k < den; ++k) {
            unsigned i = input.next_int();
            original_sc.insert_cell(i - 1, j);
        }
    }

    std::cout << "Finished building structure" << std::endl << std::endl;
    std::cout << "REDUCTIONS" << std::endl;

    SetCover reduced_sc(original_sc);

    unsigned deleted_elements;

    do {
        deleted_elements = 0;
        deleted_elements += reduced_sc.fix_essential_columns(assignment);
        deleted_elements += reduced_sc.fix_out_dominated_rows(assignment);
        deleted_elements += reduced_sc.fix_out_dominated_cols(assignment);
        

        reduced_sc.delete_fix_out_rows(assignment);
        reduced_sc.delete_fix_out_cols(assignment);
        std::cout << "Remaining rows: " << reduced_sc.remaining_rows() << std::endl << "Remaining cols: " << reduced_sc.remaining_cols() << std::endl;
        

    } while (deleted_elements != 0);

    std::cout<< std::endl << "CHVATAL" << std::endl;
    SetCover sc_chvatal(reduced_sc);
    sc_chvatal.solution_by_score(assignment);
    std::cout << "Solution cost before Chvatal reduction: " << original_sc.solution_value(assignment) << std::endl;
    reduced_sc.chvatal_solution_red(assignment);
    std::cout << "Solution cost after Chvatal reduction: " << original_sc.solution_value(assignment) << std::endl;

    if (original_sc.solution_is_correct(assignment))
        std::cout << "Solution is correct" << std::endl;
    else 
        std::cout << "Solution is wrong, check your code" << std::endl;

    std::cout << "Solution cost: " << original_sc.solution_value(assignment) << std::endl;

    //reduced_sc.print_solution(assignment.cols);
    
    std::cout << std::endl;
    
    return 0;
}

