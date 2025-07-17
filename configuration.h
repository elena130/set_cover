#include <vector>
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
};

#endif 