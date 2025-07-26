#include <vector>
#include <queue>
#include <cmath>
#include "LagrangianData.h"
#include "status.h"
#include "setcover.h"
#include "configuration.h"

#ifndef BAB_H
#define BAB_H

struct DebugInfo {
	unsigned id;
	unsigned depht;

	DebugInfo() : id(1), depht(0) {}

	DebugInfo(const unsigned node_id, const unsigned node_depht) : id(node_id), depht(node_depht){}

	~DebugInfo(){}

	DebugInfo(const DebugInfo& di) : id(di.id), depht(di.depht){}
};

struct BranchInfo {
	unsigned col;
	// f = 1 -> FIX_IN
	// f = 2 -> FIX_OUT
	unsigned f;

	BranchInfo(const unsigned c, const unsigned f_op) : col(c), f(f_op){}

	BranchInfo(const BranchInfo& bi) : col(bi.col), f(bi.f){}

	~BranchInfo(){}

	void operator=(const BranchInfo& bi) {
		col = bi.col;
		f = bi.f;
	}
};

enum ProblemStatus {
	SOLVED,
	OPEN,
	UNSOLVABLE
};

	struct BNode {
		DebugInfo di;
		LagrangianResult lr;
		Configuration conf;
		ProblemStatus status;
		BranchInfo branch_info;

		BNode(const unsigned id, const unsigned depht, const LagrangianResult &lag_res, const Configuration& configuration, const BranchInfo &bi) : di(id, depht),  lr(lag_res), conf(configuration), status(OPEN), branch_info(bi) {}

		BNode(const BNode &n) : di(n.di), lr(n.lr), conf(n.conf), status(n.status), branch_info(n.branch_info){}

		~BNode(){}

		void operator=(const BNode& n) {
			di = n.di;
			lr = n.lr;
			status = n.status;
			branch_info = n.branch_info;
		}

		bool operator<(const BNode& other) const {
			return lr.ub > other.lr.ub;
		}
	};

class BAB {
private:
	std::priority_queue<BNode> queue;

	// TODO: inserire parametri aggiuntivi 
	// - numero massimo di nodi 
	// - tempo massimo trascorso
	// - UB e LB 
	LagrangianResult bounds;
	ProblemStatus status;

public:
	BAB(LagrangianResult &lr);

	~BAB();

	LagrangianResult branching(SetCover& ref_sc, LagrangianResult& b);

	void process_bnode(BNode& father, const SetCover& original);

	void insert_bnode(const BNode& node);

	BNode extract_bnode();

	bool useful_bnode(const BNode& node);

	void derive_bnode(const BNode& father, BNode son, unsigned f);
};

#endif