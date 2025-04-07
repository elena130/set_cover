#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>

using namespace std;

class Reader {
private:
    ifstream file;
    string line;
    char* line_ptr;
    const char* sep;
public: 
    Reader(const string file_name, const char* s)  {
        sep = s;
        line_ptr = NULL;
        file.open(file_name);
        if (file.fail()) {
            cout << "Error while opening file" << endl;
            return;
        }
        else {
            advance_line();
        }
    }

    void advance_line() {
        getline(file, line);
        line_ptr = &(line[0]);
    }

    int next_int() {
        int val = stoul(strtok(line_ptr, sep));
        line_ptr = NULL;
        return val;
    }

    void close() {
        file.close();
    }
};

class Cell {
public:
    unsigned row, col;
    Cell* up;
    Cell* right;
    Cell* down;
    Cell* left;

    Cell() : row(0), col(0), up(), right(), down(), left() { }

    Cell(const unsigned r, const unsigned c) : row(r), col(c) {
        Cell();
    }
};

class SetCover {
private:
    std::vector<Cell*> rows;
    std::vector<Cell*> cols;
    std::vector<int> costs;
    std::vector<unsigned> row_density;
    std::vector<unsigned> col_density;

public:
    SetCover(int r, int c) : rows(r), cols(c), costs(c), row_density(r), col_density(c,0) {}

    void insert_cell(const unsigned i, const unsigned j) {
        Cell* c = new Cell(i,j);
        Cell* tail = cols[j] == NULL ? NULL : cols[j]->up;
        
        if (tail == NULL) {
            cols[j] = c;
            cols[j]->down = cols[j];
            cols[j]->up = cols[j];
        }
        else {
            tail->down = c;
            c->up = tail;
            c->down = cols[j];
            cols[j]->up = c;
        }

        if (rows[i] == NULL) {
            rows[i] = c;
            c->right = c;
            c->left = c;
        }
        else {
            Cell* prec = rows[i]->left;
            prec->right = c;
            c->left = prec;
            c->right = rows[i];
            rows[i]->left = c;
        }
        
    }

    void set_cost(const unsigned j,  const unsigned cost) {
        costs[j] = cost;
    }

    void inc_row_den(const unsigned i) {
        row_density[i]++;
    }

    void set_col_den(const unsigned j, const unsigned den) {
        col_density[j] = den;
    }

    unsigned get_col_den(const unsigned j) {
        return col_density[j];
    }
};


void trim_file(string file_name, string out_name) {
    Reader p(file_name, " ");
    std::ofstream outfile (out_name);

    if (outfile.fail()) {
        cout << "Error while opening file: " << out_name << endl;
        return;
    }

    unsigned rows = p.next_int();
    unsigned cols = p.next_int();
    p.advance_line();

    outfile << rows << " " << cols << endl;


    for (unsigned j = 0; j < cols; ++j) {
        unsigned cost = p.next_int();
        outfile << cost << " ";
        unsigned den = p.next_int();
        outfile << den;
        std::vector<unsigned> r(den, 0);
        for (unsigned i = 0; i < den; ++i) {
            r[i] = p.next_int() - 1;
        }
        std::sort(r.begin(), r.end());
        for (unsigned i = 0; i < den; ++i) {
            outfile << " " << r[i];
        }
        outfile << endl;
        p.advance_line();
    }

    outfile.close();
    p.close();
}

int main() {
    fstream file;
    string line;
    unsigned int nr, nc;

    // string orig_file = "rail516.txt"
    string orig_file = "C:\\Users\\Elena\\Documents\\Tesi\\codice\\data\\rail507.txt";
    //string clean_file = "clean_" + orig_file;
    string clean_file = "C:\\Users\\Elena\\Documents\\Tesi\\codice\\clean_data\\rail507.txt";

    trim_file(orig_file, clean_file);

    cout << "Reading matrix" << endl;
    Reader parser(clean_file, " ");

    nr = parser.next_int();
    nc = parser.next_int();
    parser.advance_line();
    cout << nr << "x" << nc << endl;

    SetCover sc(nr, nc);

    for (unsigned j = 0; j < nc; ++j) {
        sc.set_cost(j, parser.next_int());
        sc.set_col_den(j, parser.next_int());
        for (unsigned k = 0; k < sc.get_col_den(j); ++k) {
            unsigned i = parser.next_int();
            sc.inc_row_den(i);
            sc.insert_cell(i, j);
        }
        parser.advance_line();
       if (j % 5000 == 0)
           cout << j << " ";
    }
    cout << "Quasi finito, controlla";

    parser.close();

    return 0;
}

