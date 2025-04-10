#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"
#include "cell.h"
#include "setcover.h"

int main() {
    std::fstream file;
    std::string line;
    unsigned int nr, nc;

    // string orig_file = "rail516.txt"
    std::string orig_file = "C:\\Users\\Elena\\Documents\\Tesi\\codice\\data\\rail4872.txt";

    std::cout << "Reading matrix" << std::endl;
    Reader input(orig_file);

    nr = input.next_int();
    nc = input.next_int();
    std::cout << nr << "x" << nc << std::endl;

    SetCover sc(nr, nc);
    std::vector<unsigned> cov_rows;

    for (unsigned j = 0; j < nc; ++j) {
        sc.set_cost(j, input.next_int());
        sc.set_col_den(j, input.next_int());
        cov_rows.resize(sc.get_col_den(j));
        for (unsigned k = 0; k < sc.get_col_den(j); ++k) {
            unsigned i = input.next_int();
            sc.inc_row_den(i-1);
            cov_rows[k] = i;
        }

        std::sort(cov_rows.begin(), cov_rows.end());

        for (unsigned k = 0; k < sc.get_col_den(j); ++k) {
            sc.insert_cell(cov_rows[k] - 1, j);
        }
    }

    return 0;
}

