#include "BAB.h"

BAB::BAB(LagrangianResult& lr) : bounds(lr){
	status = OPEN;
}

BAB::~BAB(){}

LagrangianResult BAB::branching(SetCover& ref_sc, LagrangianResult& b) {

	BNode father(1, 0, b, ref_sc.get_configuration(), BranchInfo(0, 0));
	long examined_nodes, removed_nodes, waiting_nodes;
	unsigned id = 1;

	process_bnode(father, ref_sc);
	examined_nodes = 1;

	insert_bnode(father);
	waiting_nodes = 1;
	removed_nodes = 0;

	while (!queue.empty() && status != SOLVED) {

		father = extract_bnode();
		--waiting_nodes;

		if (useful_bnode(father)) {
			// only two sons
			for (unsigned f = 1; f <= 2; ++f) {
				++id;
				BNode son(id, father.di.depht + 1, father.lr, father.conf, BranchInfo(father.branch_info.col, f));
				derive_bnode(father, son, f);
				process_bnode(son, ref_sc);
				++examined_nodes;

				if (useful_bnode(son)) {
					insert_bnode(son);
					++waiting_nodes;
				}
			}
		}
		++removed_nodes;
	}

	return bounds;
}

void BAB::process_bnode(BNode& node, const SetCover& original) {
	SetCover sc(original);
	sc.change_configuration(node.conf);

	for (const Cell* c : sc.col(node.branch_info.col)) {
		if (sc.get_row_den(c->row) == 1 && node.branch_info.f == 2) {
			node.status = UNSOLVABLE;
			return;
		}
	}

	std::vector<bool> modified_rows(sc.number_of_rows(), false);
	std::vector<bool> modified_cols(sc.number_of_cols(), false);
	sc.delete_fix_out_cols(modified_rows);
	sc.delete_fix_out_rows(modified_cols);

	if (!sc.can_be_solved()) {
		node.status = UNSOLVABLE;
	}

	LagrangianPar lp;
	lp.init_ub = node.lr.ub;
	lp.init_ub_sol = node.lr.ub_sol;
	lp.init_pi = 2;         // Beasley
	lp.init_t = 1;
	lp.max_iter = 1000;
	lp.min_t = 0.005;
	LagrangianResult lagrangian_res = sc.lagrangian_lb(lp);


	if (lagrangian_res.ub < bounds.ub && lagrangian_res.ub >= bounds.lb) {
		bounds.ub = lagrangian_res.ub;
		bounds.ub_sol = lagrangian_res.ub_sol;
	}

	if (lagrangian_res.lb > bounds.lb && lagrangian_res.lb <= bounds.ub) {
		bounds.lb = lagrangian_res.lb;
		bounds.lb_sol = lagrangian_res.lb_sol;
	}

	if (bounds.lb == bounds.ub)
		status = SOLVED;

	// genera info per creare i figli 
	node.lr = lagrangian_res;

	double max = 0;
	unsigned row = 0;
	for (unsigned i = 0; i < sc.number_of_rows(); ++i) {
		if (node.conf.rows[i] != FREE)
			continue;
		double product = std::abs(node.lr.multipliers[i] * node.lr.subgradients[i]);
		if (product > max) {
			max = product;
			row = i;
		}
	}

	unsigned col = 0;
	double min_cost = 10000;
	for (const Cell* c : sc.row(row)) {
		if (node.lr.lagrangian_costs[c->col] < min_cost) {
			min_cost = node.lr.lagrangian_costs[c->col];
			col = c->col;
		}
	}

	node.branch_info.col = col;
}

void BAB::insert_bnode(const BNode& node){
	queue.push(node);
}

BNode BAB::extract_bnode(){
	BNode node = queue.top();
	queue.pop();
	return node;
}

bool BAB::useful_bnode(const BNode& node){
	return node.status == OPEN && status != SOLVED && node.lr.ub >= bounds.lb;
}

void BAB::derive_bnode(const BNode& father, BNode son, unsigned f){
	son.conf = father.conf;
	son.conf.cols[father.branch_info.col] = father.branch_info.f == 1 ?  FIX_IN : FIX_OUT;
}
