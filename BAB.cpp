#include "BAB.h"

BAB::BAB(VisitStrategy visit_strategy, LagrangianResult& lr) : strategy(visit_strategy), queue(), bounds(lr){
	status = UNKNOWN;
}

BAB::~BAB(){}

LagrangianResult BAB::branching(SetCover& ref_sc, LagrangianResult& b) {

	unsigned id, f;
	BNode * root = new BNode(1, 0, 0, ref_sc.get_configuration(), b, 0);
	root->results.multipliers = std::vector<double>(ref_sc.number_of_rows(), 0);
	long examined_nodes, removed_nodes, waiting_nodes;

	SetCover sc(ref_sc);
	process_bnode(root, sc);
	update_bounds(root->results);
	examined_nodes = 1;

	insert_bnode(root);
	waiting_nodes = 1;

	id = 1;
	removed_nodes = 0;
	while (!queue.empty() )
	{
		BNode* father = extract_bnode();
		waiting_nodes--;

		if (useful_bnode(father, ref_sc))
		{
			for (f = 1; f <= 2; f++)
			{
				id++;
				BNode *son = new BNode(id, father->level + 1, f, father->data, father->results, 0);
				SetCover sc(ref_sc);
				derive_bnode(father, son, f, sc);
				derive_set_cover(sc, son->data);
					
				process_bnode(son, sc);
				examined_nodes++;

				if (useful_bnode(son, ref_sc)) {
					insert_bnode(son);
					waiting_nodes++;
					update_bounds(son->results);
				}
				else {
					delete son;
				}
			}
		}
		delete father;
		removed_nodes++;
	}

	return bounds;
}

// cancella il nodo dopo che hai finito di usarlo 
void BAB::process_bnode(BNode* node, SetCover& sc) {

	LagrangianPar lp;
	lp.init_ub = node->results.ub;
	lp.init_ub_sol = node->results.ub_sol;
	lp.init_pi = 2;         // Beasley
	lp.init_t = 1;
	lp.max_iter = 1000;
	lp.min_t = 0.005;

	LagrangianVar lv;
	lv.cost_lagrang = std::vector<double>(sc.number_of_cols());
	lv.ub = lp.init_ub;    // UB
	lv.lb = 0;  // LB
	lv.pi = lp.init_pi;
	lv.solution = std::vector<bool>(sc.number_of_cols(), false);    // solution vector 
	lv.t = lp.init_t;
	lv.subgradients = std::vector<int>(sc.number_of_rows());     // G_i
	lv.prec_direction = std::vector<double>(sc.number_of_rows(), 0);
	lv.direction = std::vector<double>(sc.number_of_rows(), 0);
	lv.beta = 0;
	lv.multipliers = node->results.multipliers;
	lv.worsening_it = 0;

	LagrangianResult lagrangian_res = sc.lagrangian_lb(lp, lv);

	if (lagrangian_res.ub == lagrangian_res.lb) {
		node->status = SOLVED;
		status = COMPLETE;

		update_bounds(lagrangian_res);
	}

	
	// generate info to create sons
	node->results = lagrangian_res;

	double max = 0;
	unsigned row = 0;
	for (unsigned i = 0; i < sc.number_of_rows(); ++i) {
		if (node->data.rows[i] != FREE)
			continue;
		double product = std::abs(node->results.multipliers[i] * node->results.subgradients[i]);
		if (product > max) {
			max = product;
			row = i;
		}
	}

	unsigned col = 0;
	double min_cost = 10000;
	for (const Cell* c : sc.row(row)) {
		if (node->data.cols[c->col] != FREE)
			continue;
		if (node->results.lagrangian_costs[c->col] < min_cost) {
			min_cost = node->results.lagrangian_costs[c->col];
			col = c->col;
		}
	}

	node->branching_col = col;
}

void BAB::insert_bnode(BNode* node){
	switch (strategy) {
	case VisitStrategy::DFS : 
		queue.push_front(node);
		break;
	case VisitStrategy::BFS:
		queue.push_back(node);
		break;
	case VisitStrategy::BEST_FIRST:
		BNode* p = queue.top();
		BNode* end = p!= NULL ? p->prec : NULL;
		while (p!= NULL && p != end && (node->results.lb <= p->results.lb)  )
		  p = p->next;
		queue.push_before(node, p);
		break;
	}
}

BNode* BAB::extract_bnode(){
	BNode* node = queue.top();
	queue.extract_node(node);
	return node;
}

bool BAB::useful_bnode(BNode* node, SetCover& ref_sc) {
	problem_is_solvable(ref_sc, node->data);
	return node->status == OPEN && status != COMPLETE && node->results.lb < bounds.ub;
}

void BAB::derive_bnode(BNode* father, BNode* son, unsigned f, const SetCover& sc) {
	son->results.multipliers = father->results.multipliers;
	son->results.lb = father->results.lb;
	son->results.lb_sol = father->results.lb_sol;
	son->results.ub = father->results.ub;
	son->results.ub_sol = father->results.ub_sol;
	son->data = father->data;
	son->data.cols[father->branching_col] = son->son_id == 1 ? FIX_IN : FIX_OUT;

	// remove the rows covered by the fixed column, if it will be fixed in the solution
	if (f == 1) {
		for (const Cell* c : sc.col(father->branching_col)) {
			son->data.rows[c->row] = FIX_OUT;
		}
	}
}

void BAB::derive_set_cover(SetCover& sc, Configuration& conf){
	sc.change_configuration(conf);

	std::vector<bool> modified_rows(sc.number_of_rows(), false);
	std::vector<bool> modified_cols(sc.number_of_cols(), false);
	sc.delete_fix_out_cols(modified_rows);
	sc.delete_fix_out_rows(modified_cols);
}

bool BAB::problem_is_solvable(SetCover& sc, Configuration& conf) {
	
	for (unsigned i = 0; i < sc.number_of_rows(); ++i) {
		if (conf.rows[i] == FIX_OUT)
			continue;

		bool can_be_covered = false;
		for (const Cell* c : sc.row(i)) {
			if (conf.cols[c->col] != FIX_OUT) {
				can_be_covered = true;
				break;
			}
		}
		if (can_be_covered == false)
			return false;
	}
	return true;
}

void BAB::update_bounds(LagrangianResult& lr)
{
	if (lr.ub < bounds.ub && lr.ub >= bounds.lb) {
		bounds.ub = lr.ub;
		bounds.ub_sol = lr.ub_sol;
	}
}

