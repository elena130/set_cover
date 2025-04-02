#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>

using namespace std;

class Cell {
public:
    int value;
    Cell* up;
    Cell* right;
    Cell* down;
    Cell* left;

    Cell(int v) {
        value = v;
        up = NULL;
        right = NULL;
        down = NULL;
        left = NULL;
    }

    Cell() {
        Cell(0);
    }
};

void create_cols(unsigned int nc, std::vector<Cell*> &cols, unsigned int nr) {
    for (unsigned int j = 0; j < nc; j++) {
        Cell* prec = new Cell();
        cols[j] = prec;
        Cell* next;

        for(unsigned int i=1; i<nr; i++) {
            next = new Cell();
            next->up = prec;
            prec->down = next;
            prec = next;
        }

        prec->down = cols[j];
        cols[j]->up = prec;
    }
}

void create_rows(unsigned int nc, unsigned int nr, std::vector<Cell*> &cols, std::vector<Cell*> rows, vector<vector<unsigned int>> &metadata) {
    Cell* ptr_col = cols[0];
    for(unsigned int i=0; i<nr; i++) {
        rows[i] = ptr_col;
        ptr_col = ptr_col->down;
    }

    // collego in orizzontale
    for(unsigned int j=0; j<nc; j++) {
        unsigned int idx = 0;
        Cell *left_ptr = cols[j];
        Cell *right_ptr;
        if( j+1 < nc) {
            right_ptr = cols[j+1];
        } else {
            right_ptr = cols[0];
        }

        for(unsigned int i=0; i<nr; i++) {
            left_ptr->right = right_ptr;
            right_ptr->left = left_ptr;

            if(idx< metadata.at(j).size() && metadata.at(j)[idx] == i) {
                left_ptr->value = 1;
                idx ++;
            }

            left_ptr = left_ptr->down;
            right_ptr = right_ptr->down;
        }
    }
}

void trim_file(string file_name, string out_file) {
    struct stat buffer;

    if( stat(file_name.c_str(), &buffer)) {
        throw runtime_error("File "+ file_name +" does not exist");
    }

    fstream file;
    file.open(file_name);
    string line;
    std::ofstream outfile (out_file);

    while (getline(file, line)) {
        unsigned int begin = 0;
        unsigned int end = line.length() - 1;
        while( line[begin] == ' '){ begin ++; }
        while( line[end] == ' '){end --;}
        outfile << line.substr(begin, end - begin + 1) << endl;
    }

    outfile.close();
}

std::vector<unsigned int> read_metadata(string line, std::vector<int> &d_row, int *d_col, int *cost) {
    vector<unsigned int> positions;
    char* c = NULL;
    *cost = stoi(strtok_s(_strdup(line.c_str()), " ", &c));
    *d_col = stoi(strtok_s(NULL, " ", &c));

    for (int k=0; k<*d_col; k++) {
        // nel file J={1,...,n} quindi sottraggo per allineare con indici nel codice
        unsigned int row = stoul(strtok_s(NULL, " ", &c))-1;
        positions.push_back(row);
        d_row[row] ++;
    }
    // le righe coperte non sono listate in ordine nel file
    std::sort(positions.begin(), positions.end());
    return positions;
}


bool test(std::vector<Cell*> &cols, vector<vector<unsigned int>> &metadata, unsigned int nc) {
    for(unsigned int j=0; j<nc; j++) {
        Cell *ref = cols[j];
        unsigned int k=0;
        for(unsigned int i=0; i<metadata[j].size(); i++) {
            while(k < metadata[j][i]) {
                ref = ref->down;
                k++;
            }

            if( ref->value != 1) {
                cout << "Error at row: " << metadata[j][i] << " col: " << j << endl;
                return false;
            }
        }
    }
    return true;
}

int main() {
    fstream file;
    string line;
    unsigned int nr, nc;
    char* c = NULL;

    // sostituire eventualmente il nome del file
    // string orig_file = "rail516.txt"
    string orig_file = "C:\\Users\\Elena\\Documents\\Tesi\\codice\\data\\rail516.txt";
    //string clean_file = "clean_" + orig_file;
    string clean_file = "C:\\Users\\Elena\\Documents\\Tesi\\codice\\clean_data\\rail516.txt";

    trim_file(orig_file, clean_file);
    file.open(clean_file);

    cout << "Reading matrix ";
    getline(file, line);
    string del = " ";
    nr = stoul(strtok_s(_strdup(line.c_str()), del.c_str(), &c));
    nc = stoul(strtok_s(NULL, del.c_str(), &c));

    cout << nr << "x" << nc << endl;

    std::vector<Cell*> cols(nc);
    std::vector<Cell*> rows(nr);
    std::vector<std::vector<unsigned int>> metadata;
    std::vector<int> d_col (nc, 0);
    std::vector<int> d_row(nc, 0);
    std::vector<int> costs(nc,0);


    // leggi e salva i dati dal file
    for (unsigned int j = 0; j < nc; j++) {
        getline(file, line);

        metadata.push_back(read_metadata(line, d_row, &d_col[j], &costs[j]));

    }
    file.close();

    create_cols(nc, cols, nr);
    create_rows(nc, nr, cols, rows, metadata);

    if(!test(cols, metadata, nc)) {
        cout << "Error while building the structure" <<endl;
    } else {
        cout << "Structure built correctly" << endl;
    }

    return 0;
}

