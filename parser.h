#include <iostream>
#include <fstream>
#include <string>

#ifndef PARSER_H
#define PARSER_H


class Reader {
private:
    std::ifstream file;
    std::string file_name;
public:
    Reader(const std::string fn) ;

    ~Reader();

    Reader(const Reader& r) ;

    void operator=( Reader& r);

    int next_int();

    const std::string get_file_name();
};

#endif