#include "BAB.h"

BAB::BAB(VisitStrategy visit_strategy, LagrangianResult& lr) : queue(visit_strategy), bounds(lr){
	status = OPEN;
}

BAB::~BAB(){}

LagrangianResult BAB::branching(SetCover& ref_sc, LagrangianResult& b) {

	long examined_nodes, removed_nodes, waiting_nodes;
	unsigned id = 1;
	BNode father(id, 0, 0, ref_sc.get_configuration(), b, 0);

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
				BNode son(id, father.level + 1, f, father.data,  father.results, father.branching_col);
				derive_bnode(father, son, f, ref_sc);
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
	sc.change_configuration(node.data);

	std::vector<bool> modified_rows(sc.number_of_rows(), false);
	std::vector<bool> modified_cols(sc.number_of_cols(), false);
	sc.delete_fix_out_cols(modified_rows);
	sc.delete_fix_out_rows(modified_cols);

	if (!sc.can_be_solved()) {
		node.status = UNSOLVABLE;
	}

	LagrangianPar lp;
	lp.init_ub = node.results.ub;
	lp.init_ub_sol = node.results.ub_sol;
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
	node.results = lagrangian_res;

	double max = 0;
	unsigned row = 0;
	for (unsigned i = 0; i < sc.number_of_rows(); ++i) {
		if (node.data.rows[i] != FREE)
			continue;
		double product = std::abs(node.results.multipliers[i] * node.results.subgradients[i]);
		if (product > max) {
			max = product;
			row = i;
		}
	}

	unsigned col = 0;
	double min_cost = 10000;
	for (const Cell* c : sc.row(row)) {
		if (node.results.lagrangian_costs[c->col] < min_cost) {
			min_cost = node.results.lagrangian_costs[c->col];
			col = c->col;
		}
	}

	node.branching_col = col;
}

void BAB::insert_bnode(const BNode& node){
	queue.insert_node(node);
}

BNode BAB::extract_bnode(){
	return queue.extract_node();
}

bool BAB::useful_bnode(const BNode& node){
	return node.status == OPEN && status != SOLVED && node.results.ub >= bounds.lb;
}

void BAB::derive_bnode(const BNode& father, BNode &son, unsigned f, const SetCover& sc) {
	son.data = father.data;
	son.data.cols[father.branching_col] = son.son_id == 1 ? FIX_IN : FIX_OUT;

	// remove the row covered by the fixed column
	if (son.son_id == 1) {
		for (const Cell* c : sc.col(son.branching_col)) {
			son.data.rows[c->row] = FIX_OUT;
		}
	}
}
