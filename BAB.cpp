#include "BAB.h"

BAB::BAB(std::unique_ptr<IBNodeQueue>&& q, LagrangianResult& lr, BranchParameters& branch_par) :
	queue(std::move(q)), bounds(lr), status(UNKNOWN), bp(branch_par) {
}

BAB::~BAB(){}

unsigned BAB::branching(SetCover& ref_sc, LagrangianResult& b) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end;
	long long time = 0;

	bool forced_termination = false;
	unsigned lb_at_stop = 0;

	unsigned id, f;
	BNode * root = new BNode();
	root->par.multipliers = std::vector<double>(ref_sc.number_of_rows(), 0);
	root->par.lagrangian_costs = std::vector<double>(ref_sc.number_of_cols(),0);
	root->par.subgradients = std::vector<int>(ref_sc.number_of_rows(),0);
	root->par.ub = b.ub;
	root->par.ub_sol = b.ub_sol;
	root->p_conf = ref_sc.get_configuration();
	long examined_nodes, removed_nodes, waiting_nodes;

	SetCover sc(ref_sc);
	process_bnode(root, sc, ref_sc);
	update_bounds(root->par);
	examined_nodes = 1;

	insert_bnode(root);
	waiting_nodes = 1;

	id = 1;
	removed_nodes = 0;
	while (!queue->empty() &&  !forced_termination)
	{
		bounds.lb = queue->min_lb();
		BNode* father = extract_bnode();
		waiting_nodes--;

		if (useful_bnode(father, ref_sc))
		{
			for (f = 1; f <= 2; f++)
			{
				id++;
				BranchInfo bi(id, father->bi.level + 1, f);
				BNode *son = new BNode(bi, father->p_conf, father->par);
				SetCover sc(ref_sc);
				derive_bnode(father, son, f, sc);
				derive_set_cover(sc, son->p_conf);

				//std::cout << "Node " << son->bi.id << " bc: " << father->bi.b_col << " ";
				process_bnode(son, sc, ref_sc);
				update_bounds(son->par);

				if (bounds.lb == bounds.ub)
					status = COMPLETE;

				//std::cout << "[" << son->par.lb << ", " << son->par.ub << "] ";
				//std::cout << "[" << bounds.lb << ", " << bounds.ub << "]" << std::endl;
				end = std::chrono::steady_clock::now();
				time = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
				examined_nodes++;

				if (useful_bnode(son, ref_sc)) {
					insert_bnode(son);
					//bounds.lb = queue->min_lb();
					waiting_nodes++;
				}
				else {
					delete son;
				}

				if (bp.max_time != 0 && time > bp.max_time) {
					forced_termination = true;
					lb_at_stop = queue->min_lb();
					break;
				}
			}
		}
		delete father;
		removed_nodes++;
	}

	b = bounds;
	b.lb = lb_at_stop;

	return examined_nodes;
}

// cancella il nodo dopo che hai finito di usarlo 
void BAB::process_bnode(BNode* node, SetCover& sc, SetCover& ref_sc) {

	LagrangianPar lp;
	lp.init_ub = node->par.ub;
	lp.init_ub_sol = node->par.ub_sol;
	lp.init_lb = node->par.lb;
	lp.init_lb_sol = node->par.lb_sol;
	lp.init_pi = 2;         // Beasley
	lp.init_t = 1;
	lp.max_iter = 1000;
	lp.min_t = 0.005;

	LagrangianVar lv;
	lv.ub = node->par.ub;
	lv.cost_lagrang = node->par.lagrangian_costs;
	lv.lb = node->par.lb;  // LB
	lv.pi = lp.init_pi;
	lv.solution = std::vector<bool>(sc.number_of_cols(), false);    // solution vector 
	lv.t = lp.init_t;
	lv.subgradients = node->par.subgradients;     // G_i
	lv.prec_direction = std::vector<double>(sc.number_of_rows(), 0);
	lv.direction = std::vector<double>(sc.number_of_rows(), 0);
	lv.beta = 0;
	lv.multipliers = node->par.multipliers;
	lv.worsening_it = 0;

	LagrangianResult lagrangian_res = sc.lagrangian_lb(lp, lv);

	if (!problem_is_solvable(ref_sc, node->p_conf)) {
		node->status = UNSOLVABLE;
	}
	else if (lagrangian_res.ub == lagrangian_res.lb) {
		node->status = SOLVED;
	}

	if (bounds.lb == bounds.ub) {
		status = COMPLETE;
	}

	// generate info to create sons
	node->par = lagrangian_res;
	node->p_conf = sc.get_configuration();
	branch_column(sc, node);
}

void BAB::branch_column(SetCover& sc, BNode* node){
	double max = 0;
	unsigned row = 0;
	for (unsigned i = 0; i < sc.number_of_rows(); ++i) {
		if (node->p_conf.rows[i] != FREE)
			continue;
		double product = std::abs(node->par.multipliers[i] * node->par.subgradients[i]);
		if (product > max) {
			max = product;
			row = i;
		}
	}

	unsigned col = 0;
	double min_cost = 10000;
	for (const Cell* c : sc.row(row)) {
		if (node->p_conf.cols[c->col] != FREE)
			continue;
		if (node->par.lagrangian_costs[c->col] < min_cost) {
			min_cost = node->par.lagrangian_costs[c->col];
			col = c->col;
		}
	}

	node->bi.b_col = col;
}

void BAB::insert_bnode(BNode* node){
	queue->push(node);
}

BNode* BAB::extract_bnode(){
	return queue->pop();
}

bool BAB::useful_bnode(BNode* node, SetCover& ref_sc) {
	return node->status == OPEN && status != COMPLETE && node->par.lb < bounds.ub;
}

void BAB::derive_bnode(BNode* father, BNode* son, unsigned f, const SetCover& sc) {
	son->par.multipliers = father->par.multipliers;
	son->par.subgradients = father->par.subgradients;
	son->par.lagrangian_costs = father->par.lagrangian_costs;
	son->par.lb = father->par.lb;
	son->par.lb_sol = father->par.lb_sol;
	son->par.ub = father->par.ub;
	son->par.ub_sol = father->par.ub_sol;
	son->p_conf = father->p_conf;
	son->p_conf.cols[father->bi.b_col] = son->bi.son_id == 1 ? FIX_IN : FIX_OUT;

	// remove the rows covered by the fixed column, if it will be fixed in the solution
	if (f == 1) {
		for (const Cell* c : sc.col(father->bi.b_col)) {
			son->p_conf.rows[c->row] = FIX_OUT;
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

