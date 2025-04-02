#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
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
    }

    Cell() {
        value = 0;
    }
};

void create_cols(int nc, std::vector<Cell*> &cols, int nr) {
    for (int j = 0; j < nc; j++) {
        Cell* prec = new Cell();
        cols[j] = prec;
        Cell* next;

        for(int i=1; i<nr; i++) {
            next = new Cell();
            next->up = prec;
            prec->down = next;
            prec = next;
        }

        prec->down = cols[j];
        cols[j]->up = prec;
    }
}

void create_rows(int nc, int nr, std::vector<Cell*> &cols, std::vector<Cell*> rows, vector<vector<int>> &metadata) {
    Cell* ptr = cols[0];
    for(int i=0; i<nr; i++) {
        rows[i] = ptr;
        ptr = ptr->down;
    }

    // collego in orizzontale
    for(int j=0; j<nc; j++) {
        int ptr = 0;
        Cell *left_ptr = cols[j];
        Cell *right_ptr;
        if( j+1 < nc) {
            right_ptr = cols[j+1];
        } else {
            right_ptr = cols[0];
        }

        for(int i=0; i<nr; i++) {
            left_ptr->right = right_ptr;
            right_ptr->left = left_ptr;

            if(ptr< metadata.at(j).size() && metadata.at(j)[ptr] == i) {
                left_ptr->value = 1;
                ptr ++;
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
        int begin = 0;
        int end = line.length() - 1;
        while( line[begin] == ' '){ begin ++; }
        while( line[end] == ' '){end --;}
        outfile << line.substr(begin, end - begin + 1) << endl;
    }

    outfile.close();
}

std::vector<int> read_metadata(string line, std::vector<int> &d_row, int *d_col, int *cost) {
    vector<int> positions;
    *cost = stoi(strtok(strdup(line.c_str()), " "));
    *d_col = stoi(strtok(NULL, " "));

    for (int k=0; k<*d_col; k++) {
        // nel file J={1,...,n} quindi sottraggo per allineare con indici nel codice
        int row = stoi(strtok(NULL, " "))-1;
        positions.push_back(row);
        d_row[row] ++;
    }
    // le righe coperte non sono listate in ordine nel file
    std::sort(positions.begin(), positions.end());
    return positions;
}


bool test(std::vector<Cell*> &cols, vector<vector<int>> &metadata, int nc) {
    for(int j=0; j<nc; j++) {
        Cell *ref = cols[j];
        int k=0;
        for(int i=0; i<metadata[j].size(); i++) {
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
    int nr, nc;

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
    nr = stoi(strtok(strdup(line.c_str()), del.c_str()));
    nc = stoi(strtok(NULL, " "));

    cout << nr << "x" << nc << endl;

    std::vector<Cell*> cols(nc);
    std::vector<Cell*> rows(nr);
    std::vector<std::vector<int>> metadata;
    std::vector<int> d_col (nc, 0);
    std::vector<int> d_row(nc, 0);
    std::vector<int> costs(nc,0);


    // leggi e salva i dati dal file
    for (int j = 0; j < nc; j++) {
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

