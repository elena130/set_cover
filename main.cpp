#include <algorithm>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <filesystem>
#include <vector>
#include "Cella.h"

class Cella;
using namespace std;

class ColMetadata {
public:
    vector<int> pos;
    ColMetadata() {}
};


void create_cols(int nc, Cella *cols[], int nr) {
    for (int j = 0; j < nc; j++) {
        Cella* prec = new Cella();
        cols[j] = prec;
        Cella* next;

        for(int i=1; i<nr; i++) {
            next = new Cella();
            next->up = prec;
            prec->down = next;
            prec = next;
        }

        prec->down = cols[j];
        cols[j]->up = prec;
    }
}

void create_rows(int nc, int nr, Cella *cols[], Cella *rows[], vector<ColMetadata> metadata) {
    Cella *ptr = cols[0];
    for(int i=0; i<nr; i++) {
        rows[i] = ptr;
        ptr = ptr->down;
    }

    // collego in orizzontale
    for(int j=0; j<nc; j++) {
        int ptr = 0;
        Cella *left_ptr = cols[j];
        Cella *right_ptr;
        if( j+1 < nc) {
            right_ptr = cols[j+1];
        } else {
            right_ptr = cols[0];
        }

        for(int i=0; i<nr; i++) {
            left_ptr->right = right_ptr;
            right_ptr->left = left_ptr;

            if(ptr< metadata.at(j).pos.size() && metadata.at(j).pos[ptr] == i) {
                left_ptr->value = 1;
                ptr ++;
            }

            left_ptr = left_ptr->down;
            right_ptr = right_ptr->down;
        }
    }
}

void trim_file(string file_name, string out_file) {
    if( !std::filesystem::exists(file_name)) {
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

ColMetadata read_metadata(string line, int d_row[], int *d_col, int *cost) {
    ColMetadata metadata;
    *cost = stoi(strtok(strdup(line.c_str()), " "));
    *d_col = stoi(strtok(NULL, " "));

    for (int k=0; k<*d_col; k++) {
        // nel file J={1,...,n} quindi sottraggo per allineare con indici della matrice
        int row = stoi(strtok(NULL, " "))-1;
        metadata.pos.push_back(row);
        d_row[row] ++;
    }
    std::sort(metadata.pos.begin(), metadata.pos.end());
    return metadata;
}

bool test(Cella* cols[], vector<ColMetadata> metadata, int nc) {
    for(int j=0; j<nc; j++) {
        Cella *ref = cols[j];
        int k=0;
        for(int i=0; i<metadata[j].pos.size(); i++) {
            while(k < metadata[j].pos[i]) {
                ref = ref->down;
                k++;
            }

            if( ref->value != 1) {
                cout << "Error at row: " << metadata[j].pos[i] << " col: " << j << endl;
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

    string file_name = "../data/rail507.txt";
    string out_file = "../clean_data/rail507.txt";

    trim_file(file_name, out_file);
    file.open(out_file);

    cout << "Reading matrix ";
    getline(file, line);
    string del = " ";
    nr = stoi(strtok(strdup(line.c_str()), del.c_str()));
    nc = stoi(strtok(NULL, " "));

    cout << nr << "x" << nc << endl;

    Cella* cols[nc];
    Cella* rows[nr];
    vector<ColMetadata> metadata;
    int d_col[nc] = {0};
    int d_row[nr] = {0};
    int costs[nc] = {0};

    // leggi e salva i dati dal file
    for (int j = 0; j < nc; j++) {
        getline(file, line);
        metadata.push_back(read_metadata(line, d_row, &d_col[j], &costs[j] ));
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

