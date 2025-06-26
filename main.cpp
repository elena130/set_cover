#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include "string.h"
#include "parser.h"
#include "setcover.h"
#include "logger.h"
#include <cmath>

double normal_score(double cost, double covered) {
    return cost / covered;
}

double log_score(double cost, double covered) {
    if (covered == 1 || covered == 2)
        return cost ;
    return cost / std::log2(covered);
}

double nlog_score(double cost, double covered) {
    if (covered == 1 || covered == 2)
        return cost;
    return cost / (covered * std::log2(covered));
}

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
        for (unsigned i = 0; i < nr; ++i)
            modified_rows[i] = false;
        sc.delete_fix_out_cols(modified_rows);

        deleted += sc.fix_out_dominated_rows(first_reduction, modified_rows);
        for (unsigned j = 0; j < nc; ++j)
            modified_cols[j] = false;
        sc.delete_fix_out_rows(modified_cols);

        first_reduction = false;
        logger.log_endl("Remaining rows: " + std::to_string(sc.remaining_rows()));
        logger.log_endl("Remaining cols: " + std::to_string(sc.remaining_cols()));
    } while (deleted != 0);

    logger.log_endl("");

    logger.log_endl("CHVATAL");

    std::set<unsigned> selected;
    Solution chvatal_sol(nc);
    Solution chvatal_log(nc);
    Solution chvatal_nlog(nc);
    sc.chvtal(chvatal_sol, normal_score);
    sc.chvtal(chvatal_log, log_score);
    sc.chvtal(chvatal_nlog, nlog_score);

    if (sc.solution_is_correct(chvatal_sol)) {
        logger.log_endl("Solution is correct");
    }
    else {
        std::cout << "Solution is wrong, check your code" << std::endl;
    }

    unsigned sol_val = sc.solution_value(chvatal_sol);
    unsigned log_sol_val = sc.solution_value(chvatal_log);
    unsigned nlog_sol_val = sc.solution_value(chvatal_nlog);
    logger.log_endl("Solution cost: " + std::to_string(sol_val));
    logger.log_endl("Solution cost log: " + std::to_string(log_sol_val));
    logger.log_endl("Solution cost nlog: " + std::to_string(nlog_sol_val));

    unsigned best_chvatal = sol_val;
    Solution best_chvatal_sol = chvatal_sol;

    if (best_chvatal > log_sol_val) {
        best_chvatal = log_sol_val;
        best_chvatal_sol = chvatal_log;
    }
    if (best_chvatal > nlog_sol_val) {
        best_chvatal = nlog_sol_val;
        best_chvatal_sol = chvatal_nlog;
    }

    LagrangianPar lp;
    lp.init_ub = best_chvatal;
    lp.init_ub_sol = best_chvatal_sol;
    lp.init_pi = 2;         // Beasley
    lp.init_t = 1;
    lp.max_iter = 1000;
    lp.min_t = 0.005;
    LagrangianResult lagrangian_res = sc.lagrangian_lb(lp);

    // opt_gap = (UB - LB) / LB * 100
    double opt_gap = ((double(lagrangian_res.ub) - lagrangian_res.lb) / lagrangian_res.lb) * 100;
    unsigned dynamic_lb = sc.dynamic_prog(lagrangian_res.multipliers, lagrangian_res.ub, lagrangian_res.lb);
    logger.log_endl("LB Dinamico: " + std::to_string(dynamic_lb));

   
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    
    logger.log_endl("Time difference [s] = " + std::to_string(time));
    
    std::cout << nr << "\t" << nc << "\t";
    std::cout << sc.remaining_rows() << "\t" << sc.remaining_cols() << "\t" << lagrangian_res.ub << "\t" << lagrangian_res.lb << "\t"  << opt_gap << "\t" << time << "\t" << dynamic_lb << std::endl;

    return 0;
}

