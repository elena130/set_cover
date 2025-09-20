#include <iostream>
#include <string>
#include <algorithm>
#include <climits>
#include <chrono>
#include <iomanip>
#include <cfloat>
#include "string.h"
#include "parser.h"
#include "setcover.h"
#include "logger.h"
#include <cmath>
#include "BAB.h"

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

unsigned char_to_unsigned(const char* s) {
    errno = 0;
    char* end;
    unsigned long val = strtoul(s, &end, 10);

    if (errno == ERANGE || *end != '\0' || end == s || val > UINT_MAX) {
        std::cerr << "Valore non valido o fuori da range unsigned int\n";
        return 1;
    }

    return static_cast<unsigned int>(val);
}

double char_to_double(const char* s) {
    errno = 0;
    char* end;
    double val = strtod(s, &end);

    if (errno == ERANGE || *end != '\0' || end == s) {
        std::cerr << "Valore non valido o fuori da range double\n";
        return 0.0;
    }

    return val;
}

void parse_parameters(const int argc, char* argv[], std::string &file_name, bool &print, BranchParameters &bp) {
    print = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--file" && i + 1 < argc) {
            file_name = argv[++i];
        }
        else if (arg == "--print") {
            print = true;
        }
        else if (arg == "--time" && i + 1 < argc) {
            bp.max_time = char_to_unsigned(argv[++i]);
        }
        else if(arg == "--min-pi" && i + 1 < argc) {
            bp.min_pi = char_to_double(argv[++i]);
        }
        else if (arg == "--init-pi" && i + 1 < argc) {
            bp.init_pi = char_to_double(argv[++i]);
        }
        else if (arg == "--worse" && i + 1 < argc) {
            bp.worsening_it = char_to_unsigned(argv[++i]);
        }
        else {
            std::cerr << "Unknown parameter: " << arg << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::fstream file;
    std::string line;
    unsigned int nr, nc;
    Logger logger;
    std::string file_name;
    bool print_flag = false;

    if (argc < 2) {
        std::cout << "Error, path to input file is not specified. Terminating" << std::endl;
        return 1;
    } 

    BranchParameters bp;
    parse_parameters(argc, argv, file_name, print_flag, bp);

    logger.set_show_prints(print_flag);
    logger.log_endl("READING MATRIX");
    Reader input(file_name);

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


    std::chrono::steady_clock::time_point begin_reduction = std::chrono::steady_clock::now();

    original_sc.logic_reductions(logger);
    SetCover sc(original_sc);

    std::chrono::steady_clock::time_point end_reduction = std::chrono::steady_clock::now();
    double reduction_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_reduction - begin_reduction).count() / 1000;

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

    LagrangianResult lr;
    lr.lb = 0;
    lr.lb_sol = Solution(sc.number_of_cols());
    lr.ub = best_chvatal;
    lr.ub_sol = best_chvatal_sol;
    BAB bab(std::make_unique<BestFirstQueue>(), lr, bp);
    LagrangianResult lr_root;
    unsigned examined_nodes = bab.branching(sc, lr, original_sc, lr_root);

    double opt_gap = 0;
    if (lr.lb != 0) {
        // opt_gap = (UB - LB) / LB * 100
        opt_gap = ((double(lr.ub) - lr.lb) / lr.lb) * 100;
    }
   
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000;
    
    logger.log_endl("Time difference [s] = " + std::to_string(time));
    
    std::cout << nr << "\t" << nc << "\t";
    std::cout << sc.remaining_rows() << "\t" << sc.remaining_cols() << "\t";
    std::cout << lr.ub << "\t" << lr.lb << "\t";
    std::cout << lr_root.ub << "\t" << lr_root.lb << "\t";
    std::cout << std::fixed << std::setprecision(2) << opt_gap << "\t";
    std::cout << examined_nodes << "\t" << reduction_time << "\t" << lr_root.time << "\t" << time << "\t" << std::endl;

    return 0;
}