#include "BAB.h"

BAB::BAB(){}

BAB::~BAB(){}

void BAB::insert_node(Node& node) {
	queue.push(node);
}

Node BAB::extract_node() {
	Node n = queue.top();
	queue.pop();
	return n;
}

const bool BAB::is_useful(Node& node, SetCover& sc) {
	return node.status != UNSOLVABLE;
}

void BAB::derive_info(Node& father, Node& son) {
	son.lr = father.lr;
	son.conf = father.conf;
}

void BAB::create_scp_from_config(SetCover& sc, Configuration &config){
	sc.change_configuration(config);
	std::vector<bool> com = std::vector<bool>(sc.number_of_cols(), false);
	sc.delete_fix_out_rows(com);
	com.resize(sc.number_of_rows(), false);
	sc.delete_fix_out_cols(com);
}

void BAB::branch_rule(){
	// find the first FREE variable 
			// TODO: there is a criteria for it, find it
	for (unsigned j = 0; j < sc.number_of_cols(); ++j) {
		if (father.conf.cols[j] == FREE) {
			col_to_assign = j;
			break;
		}
	}
}

LagrangianResult BAB::branch_and_bound(SetCover& ref, const unsigned ub, Solution& ub_sol) {
	unsigned id = 0;
	Node root(1, 0);
	root.lr.lb = 0;
	root.lr.ub = ub;
	root.lr.ub_sol = ub_sol;
	root.conf = ref.get_configuration();

	process_node(ref, root);
	insert_node(root);

	unsigned best_ub = root.lr.ub;
	Solution best_ub_sol = root.lr.ub_sol;
	unsigned best_lb = root.lr.lb;
	Solution best_lb_sol = root.lr.lb_sol;
	bool solved = best_ub == best_lb;

	while (!queue.empty() && !solved) {
		Node father = extract_node();
		SetCover sc(ref);
		create_scp_from_config(sc, father.conf);

		unsigned col_to_assign = 0;

		if (is_useful(father, sc)) {
			

			bool fix_in = true;
			for (unsigned i = 0; i < 2; i++) {
				Node son(++id, father.di.depht + 1);
				derive_info(father, son);
				son.conf.cols[col_to_assign] = (fix_in) ? FIX_IN : FIX_OUT;
				
				if (!fix_in) {
					std::vector<bool> com = std::vector<bool>(sc.number_of_rows(), true);
					sc.remove_col(col_to_assign, com);
				}

				process_node(sc, son);

				if (is_useful(son, sc)) {
					insert_node(son);

					if (son.lr.lb > best_lb) {
						best_lb = son.lr.lb;
						best_lb_sol = son.lr.lb_sol;
					}
					if (son.lr.ub < best_ub) {
						best_ub = son.lr.ub;
						best_ub_sol = son.lr.ub_sol;
					}

					if (son.status == SOLVED) {
						solved = true;
						break;
					}
				}
				fix_in = false;
			}
		}
	}

	LagrangianResult lr;
	lr.ub = best_ub;
	lr.ub_sol = best_ub_sol;
	lr.lb = best_lb;
	lr.lb_sol = best_lb_sol;

	return lr;
}

void BAB::process_node(SetCover& ref, Node& node) {

	// run subgradient optimization 
	LagrangianPar lp;
	lp.init_ub = node.lr.ub;
	lp.init_ub_sol = node.lr.ub_sol;
	lp.init_pi = 2;         // Beasley
	lp.init_t = 1;
	lp.max_iter = 1000;
	lp.min_t = 0.005;
	LagrangianResult lagrangian_res = ref.lagrangian_lb(lp);

	if (lagrangian_res.lb == lagrangian_res.ub) {
		node.status = SOLVED;
	}
	/*
	else if(!ref.can_be_solved()){
		node.status = UNSOLVABLE;
	}
	*/
	node.lr.lagrangian_costs = lagrangian_res.lagrangian_costs;
	node.lr.lb = lagrangian_res.lb;
	node.lr.lb_sol = lagrangian_res.lb_sol;
	node.lr.multipliers = lagrangian_res.multipliers;
	node.lr.ub = lagrangian_res.ub;
	node.lr.ub_sol = lagrangian_res.ub_sol;
}