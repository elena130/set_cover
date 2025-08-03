#include <vector>
#include <queue>
#include <cmath>
#include <memory>
#include "LagrangianData.h"
#include "status.h"
#include "setcover.h"
#include "configuration.h"
#include "VisitStrategy.h"

#ifndef BAB_H
#define BAB_H

enum ProblemStatus {
	SOLVED,
	OPEN,
	UNSOLVABLE
};

struct BNode {
	unsigned id, level, son_id;
	Configuration data;
	LagrangianResult results;
	unsigned branching_col;
	ProblemStatus status;

	BNode(const unsigned i, const unsigned lv, const unsigned s_id, const Configuration& d,
		const LagrangianResult& lr, const unsigned branch_c) :
	id(i), level(lv), son_id(s_id), data(d), results(lr), branching_col(branch_c), status(OPEN)
	{}

	~BNode(){}

	BNode(const BNode& n): id(n.id), level(n.level), son_id(n.son_id), data(n.data), results(n.results),
	branching_col(n.branching_col), status(n.status){}

	void operator=(const BNode& n) {
		id = n.id;
		level = n.level;
		results = LagrangianResult(n.results);
		branching_col = n.branching_col;
		status = n.status;
	}
};



struct NodeComparator {
	VisitStrategy strategy;

	NodeComparator(VisitStrategy s) : strategy(s) {}

	bool operator()(const BNode& a, const BNode& b) const {
		switch (strategy) {
		case VisitStrategy::DFS:
			return a.level < b.level; // DFS: nodi più profondi prima
		case VisitStrategy::BFS:
			return a.level > b.level; // BFS: nodi più vicini prima
		case VisitStrategy::BEST_FIRST:
			return a.results.ub > b.results.ub; 
		default:
			return false;
		}
	}
};


class BNodeQueue {
private:
	VisitStrategy strategy;
	NodeComparator comp;
	using QueueType = std::priority_queue<BNode, std::vector<BNode>, NodeComparator>;
	QueueType queue;

public:
	BNodeQueue(VisitStrategy s) : strategy(s), comp(s), queue(comp) {}

	void insert_node(const BNode& node) {
		queue.push(node); 
	}

	BNode extract_node() {
		BNode n = queue.top();
		queue.pop();
		return n; 
	}

	bool empty() const {
		return queue.empty();
	}
};


class BAB {
private:
	BNodeQueue queue;
	LagrangianResult bounds;
	ProblemStatus status;

public:
	BAB(VisitStrategy visit_strategy, LagrangianResult &lr);

	~BAB();

	LagrangianResult branching(SetCover& ref_sc, LagrangianResult& b);

	void process_bnode(BNode& father, SetCover& sc);

	void insert_bnode(const BNode& node);

	BNode extract_bnode();

	bool useful_bnode(const BNode& node);

	void derive_bnode(const BNode& father, BNode &son, unsigned f, const SetCover &sc);

	void derive_set_cover(SetCover& sc, Configuration &conf);
};

#endif