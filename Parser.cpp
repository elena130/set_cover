#include <iostream>
#include <string>
#include <fstream>
#include "Parser.h"


Reader::Reader(const std::string fn) : file_name(fn) {
    file.open(fn, std::ifstream::in);
    if (file.fail()) {
        std::cout << "Error while opening file" << std::endl;
        return;
    }
}

Reader::~Reader() {
    file.close();
}

Reader::Reader(const Reader& r) : file_name(r.file_name) {
    file.open(r.file_name, std::ifstream::in);
}

void Reader::operator=( Reader& r){
    if (file.is_open()) {
        file.close();
    }

    file_name = r.file_name;
    file.open(file_name);
    file.seekg(r.file.tellg());
}

int Reader::next_int() {
    unsigned val;
    file >> val;
    return val;
}

