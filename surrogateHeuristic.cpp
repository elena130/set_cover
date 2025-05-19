#include <iostream>
#include <vector>
#include "setcover.h"
#include <float.h>
#include <algorithm>

struct LagrangeanVar {
	double ub;			// upper bound
	double lb;			// lower bound 
	unsigned j_star;				// j* 
	double t;			// step size 
	std::vector<double> omega;		// weights of each constraint
	std::vector<double> solution;
};

bool cmp_cres(const std::pair<double, unsigned> & a, const std::pair<double, unsigned> & b) {
	if (a.first < b.first) {
		return true;
	}
	else if (a.first == b.first && a.second < b.second) {
		return true;
	}
	else {
		return false;
	}
}

void SetCover::surrogate_heuristic() {
	LagrangeanVar lv;
	lv.ub = DBL_MAX;
	lv.lb = DBL_MIN;
	lv.omega.resize(n_rows, 1);
	lv.solution.resize(n_cols, 0);
	lv.j_star = 0;


	std::vector<std::pair<double, unsigned>> scores;
	calculate_scores(lv, scores);
	
}

void SetCover::find_j_star(LagrangeanVar lv, std::vector<std::pair<double, unsigned>> & scores) {
	double we = 0;
	double prec_sum = 0;
	double cur_sum = 0;

	for (double o : lv.omega) {
		we += o;
	}

	for (unsigned j : available_col) {
		if (j < lv.j_star -1) {
			prec_sum = cur_sum;
			break;
		}
		Cell* ptr = cols[j];
		for (unsigned k = 0; k < col_density[j]; ++k) {
			cur_sum += lv.omega[ptr->row];
			ptr = ptr->down;
		}
	}

	bool ok = true;

	while (ok) {
		if (cur_sum < we) {
			std::set<unsigned>::iterator it = available_col.find(lv.j_star);
			lv.j_star = *(++it);
			prec_sum = cur_sum;
			Cell* ptr;
			for (unsigned k = 0; k < col_density[lv.j_star]; ++k) {
				cur_sum += lv.omega[ptr->row];
				ptr = ptr->down;
			}
		}
		else if( prec_sum > we ){
			std::set<unsigned>::iterator it = available_col.find(lv.j_star);
			lv.j_star = *(--it);
			cur_sum = prec_sum;
			Cell* ptr = cols[lv.j_star];
			for (unsigned k = 0; k < col_density[lv.j_star]; ++k) {
				prec_sum -= lv.omega[ptr->row];
				ptr = ptr->down;
			}
		}
		else {
			ok = false;
		}
	}
}

void SetCover::calculate_scores(LagrangeanVar lv, std::vector<std::pair<double,unsigned>> & scores) {
	for (unsigned j : available_col) {
		double sum = 0;
		Cell* col = cols[j];
		for (unsigned k = 0; k < col_density[j]; ++k) {
			sum += lv.omega[col->row];
		}
		scores.push_back(std::make_pair(double(costs[j]) / sum, j));
	}

	std::sort(scores.begin(), scores.end(), cmp_cres);
}