#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include "parser.h"
#include "setcover.h"
#include "status.h"

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
        sc.set_cost(j, input.next_int());
        unsigned den = input.next_int();
        for (unsigned k = 0; k < den; ++k) {
            unsigned i = input.next_int();
            sc.insert_cell(i - 1, j);
        }
    }

    std::cout << "Finished building structure" << std::endl;

    std::vector<Status> stat(nc, FREE); 
    std::vector<Status> row_stat(nr, FREE);
    unsigned fixed_cols = 0;

    for (unsigned i = 0; i < nr; ++i) {
        if (sc.get_row_den(i) == 1) {
            stat[sc.get_row_head(i)->col] = FIX_IN;
            ++fixed_cols;
        }
    }

    std::cout << "Fixed columns: " << fixed_cols << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned i = 0; i < nr; ++i) {
        for (unsigned k = 1; k < nr; ++k) {
            bool res = sc.is_subset(i, k);
            if (res) {
                row_stat[k] = FIX_OUT;
            }
        }

        if (i % 500 == 0) {
            std::cout << i << "\t";
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Found dominant rows in "<< std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << std::endl;

    return 0;
}

