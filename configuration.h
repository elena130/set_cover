#include <vector>
#include <iostream>
#include "status.h"

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

struct Configuration {
	std::vector<Status> rows;
	std::vector<Status> cols;

	Configuration(){}

	Configuration(const unsigned n_rows, const unsigned n_cols) : rows(n_rows, FREE), cols(n_cols, FREE) {}

	Configuration(const Configuration& conf) : rows(conf.rows), cols(conf.cols){}

	~Configuration(){}

	void clear() {
		rows.clear();
		cols.clear();
	}

	void print() {
		std::cout << "Col: " << std::endl;
		for (unsigned j = 0; j < cols.size(); ++j) {
			std::cout << j << "\t" << cols[j] << std::endl;
		}

		std::cout << "Righe: " << std::endl;
		for (unsigned i = 0; i < rows.size(); ++i) {
			std::cout << i << "\t" << rows[i] << std::endl;
		}
	}
};

#endif 