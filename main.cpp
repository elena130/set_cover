#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include "string.h"
#include "parser.h"
#include "setcover.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::fstream file;
    std::string line;
    unsigned int nr, nc;
    Logger logger;

    if (argc < 2) {
        std::cout << "Error, path to input file is not specified. Terminating" << std::endl;
        return 1;
    } 

    if (argc == 3)
        logger.set_show_prints( strcmp(argv[2],"p") == 0);

    logger.log_endl("READING MATRIX");
    Reader input(argv[1]);

    nr = input.next_int();
    nc = input.next_int();
    logger.log_endl(std::to_string(nr) + "x" + std::to_string(nc));

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

    logger.log_endl("Finished building structure");
    logger.log_endl("");
    logger.log_endl("REDUCTIONS");

    SetCover sc(original_sc);
    std::vector<bool> modified_cols(nc, false);
    std::vector<bool> modified_rows(nr, false);
    unsigned deleted;
    bool first_reduction = true;

    do {
        deleted = 0;
        
        deleted += sc.fix_essential_columns(first_reduction, modified_rows);
        deleted += sc.fix_out_cols_dom_set(first_reduction, modified_cols);
        deleted += sc.fix_out_dominated_cols(first_reduction, modified_cols, logger);
        std::fill_n(modified_rows.begin(), nr, false);
        sc.delete_fix_out_cols(modified_rows);

        deleted += sc.fix_out_dominated_rows(first_reduction, modified_rows);
        std::fill_n(modified_cols.begin(), nc, false);
        sc.delete_fix_out_rows(modified_cols);

        first_reduction = false;
        logger.log_endl("Remaining rows: " + std::to_string(sc.remaining_rows()));
        logger.log_endl("Remaining cols: " + std::to_string(sc.remaining_cols()));
    } while (deleted != 0);

    logger.log_endl("");
    logger.log_endl("CHVATAL");

    std::set<unsigned> selected;
    Solution chvatal_sol(nc);
    sc.chvtal(chvatal_sol);

    if (sc.solution_is_correct(chvatal_sol)) {
        logger.log_endl("Solution is correct");
    }
    else {
        std::cout << "Solution is wrong, check your code" << std::endl;
    }

    unsigned sol_val = sc.solution_value(chvatal_sol);
    logger.log_endl("Solution cost: " + std::to_string(sol_val));

    

    LagrangianPar lp;
    lp.ub = sc.solution_value_without_fixed_in(chvatal_sol);
    lp.init_pi = 0.1;         // Beasley
    lp.init_t = 1;
    lp.max_iter = 300;
    lp.min_t = 0.005;
    lp.min_diff = 0.005;
    LagrangianRes lr = sc.LagrangianReslagrangian_lb(lp);

    sol_val = sc.solution_value(chvatal_sol);
    logger.log_endl("Chvatal updated: " + std::to_string(sol_val));
    // opt_gap = (UB - LB) / LB * 100
    double opt_gap = ((double(lr.ub) - lr.max_lb) / lr.max_lb) * 100;
    
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    
    logger.log_endl("Time difference [s] = " + std::to_string(time));
    
    std::cout << nr << "\t" << nc << "\t";
    std::cout << sc.remaining_rows() << "\t" << sc.remaining_cols() << "\t" << lr.ub << "\t" << lr.max_lb << "\t"  << opt_gap << "\t" << time << std::endl;

    return 0;
}

