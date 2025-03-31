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

void trim_file(string file_name, string out_file);

ColMetadata read_metadata(string line, int d_row[], int *d_col, int *cost) ;

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

    for (int j = 0; j < nc; j++) {
        getline(file, line);
        metadata.push_back(read_metadata(line, d_row, &d_col[j], &costs[j] ));

        Cella* prec = new Cella(j*10);

        cols[j] = prec;
        Cella* next;
        Cella* ptr_col;

        if(j>0) {
           ptr_col = cols[j-1];
            ptr_col->right = cols[j];
            cols[j]->left = ptr_col;
            ptr_col = ptr_col->down;
        }

        for(int i=1; i<nr; i++) {
            next = new Cella(j*10 + i);
            next->up = prec;
            prec->down = next;

            if(j>0) {
                next->left = ptr_col;
                ptr_col->right = next;
                ptr_col = ptr_col->down;
            }
            prec = next;
        }

        prec->down = cols[j];
        cols[j]->up = prec;
    }

    // inizializza i puntatori alle liste delle righe
    rows[0] = cols[0];
    for(int i=1; i<nr; i++) {
        rows[i] = rows[i-1]->down;
    }

    // collego elementi dell'ultima colonna circolarmente rispetto alle testse delle righe
    Cella* tail = cols[nc-1];
    for(int i=0; i<nr; i++) {
        rows[i]->left = tail;
        tail->right = rows[i];
        tail = tail->down;
    }

    for(int j=0; j<nc; j++) {
        cout << costs[j] << "\t";
    }

    file.close();
    return 0;
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
    return metadata;
}