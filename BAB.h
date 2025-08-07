#include <vector>
#include <queue>
#include <cmath>
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
	UNSOLVABLE,
	IRRELEVANT
};

enum SolutionStatus {
	UNKNOWN,
	PARTIAL,
	COMPLETE
};

struct BNode {
	unsigned id, level, son_id;
	Configuration data;
	LagrangianResult results;
	unsigned branching_col;
	ProblemStatus status;
	BNode* prec;
	BNode* next;

	BNode(): id(0), level(0), son_id(0), data(), results(), branching_col(0), status(OPEN), prec(NULL),
	next(NULL){}

	BNode(const unsigned i, const unsigned lv, const unsigned s_id, const Configuration& d,
		const LagrangianResult& lr, const unsigned branch_c) :
	id(i), level(lv), son_id(s_id), data(d), results(lr), branching_col(branch_c), status(OPEN),
		prec(NULL), next(NULL)
	{}

	~BNode(){}

	BNode(const BNode& n): id(n.id), level(n.level), son_id(n.son_id), data(n.data), results(n.results),
	branching_col(n.branching_col), status(n.status), prec(n.prec), next(n.next){}

	void operator=(const BNode& n) {
		id = n.id;
		level = n.level;
		results = LagrangianResult(n.results);
		branching_col = n.branching_col;
		data = n.data;
		status = n.status;
	}
};


class BNodeQueue {
private:
	BNode* queue;
	unsigned n;

public:
	BNodeQueue() : queue(NULL), n(0) {}

	void push_back(BNode* node) {
		++n;
		if (queue == NULL) {
			queue = node;
			node->next = node;
			node->prec = node;
			return;
		}

		BNode* l = last();
		l->next = node;
		node->prec = l;
		node->next = queue;
		queue->prec = node;
	}

	void push_front(BNode* node) {
		++n;
		if (queue == NULL) {
			queue = node;
			node->next = node;
			node->prec = node;
			return;
		}

		BNode* first = queue;
		node->next = first;
		node->prec = first->prec;
		first->prec->next = node;
		first->prec = node;
		queue = node;
	}

	void push_before(BNode* node, BNode* ref) {
		++n;

		if (queue == NULL) {
			queue = node;
			node->next = node;
			node->prec = node;
			return;
		}

		if (ref == queue) {
			queue = node;
		}

		node->next = ref;
		node->prec = ref->prec;
		ref->prec->next = node;
		ref->prec = node;
	}

	void extract_node(BNode* to_be_extracted) {
		if (n==0)
			return;
		else if (n == 1) {
			to_be_extracted = queue;
			queue = NULL;
		} else if (n > 1) {
			queue = to_be_extracted->next;
			to_be_extracted->prec->next = to_be_extracted->next;
			to_be_extracted->next->prec = to_be_extracted->prec;
		}

		--n;
	}

	BNode* top() {
		return queue;
	}

	BNode* last() {
		if (queue == NULL)
			return NULL;
		return queue->prec;
	}

	bool empty() const {
		return queue == NULL;
	}
};


class BAB {
private:
	VisitStrategy strategy;
	BNodeQueue queue;
	LagrangianResult bounds;
	SolutionStatus status;

public:
	BAB(VisitStrategy visit_strategy, LagrangianResult &lr);

	~BAB();

	LagrangianResult branching(SetCover& ref_sc, LagrangianResult& b);

	void process_bnode(BNode* node, SetCover& sc);

	void insert_bnode(BNode* node);

	BNode* extract_bnode();

	bool useful_bnode(BNode * node, SetCover& ref_sc);

	void derive_bnode(BNode * father, BNode * son, unsigned f, const SetCover &sc);

	void derive_set_cover(SetCover& sc, Configuration &conf);

	bool problem_is_solvable(SetCover & sc, Configuration& conf);

	void update_bounds(LagrangianResult &lr);
};

#endif