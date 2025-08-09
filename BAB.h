#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <cmath>
#include <chrono>
#include "LagrangianData.h"
#include "status.h"
#include "setcover.h"
#include "configuration.h"

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

struct BranchInfo {
	unsigned id, level, son_id, b_col;

	BranchInfo() : id(1), level(0), son_id(0), b_col(0) {}

	BranchInfo(const unsigned i, const unsigned l, const unsigned s_id) : id(i), level(l), 
	son_id(s_id), b_col(0){}

	BranchInfo(const BranchInfo& b) : id(b.id), level(b.level), son_id(b.son_id), b_col(b.b_col) {}

	~BranchInfo(){}

	void operator=(const BranchInfo& b) {
		id = b.id;
		level = b.level;
		son_id = b.son_id;
		b_col = b.b_col;
	}

	// TODO: mettere stampa per i dati 
};

struct BNode {
	BranchInfo bi;					// Info necessary for branching 
	Configuration p_conf;			// problem configuration 
	LagrangianResult par;			// parameters of subgradient optimization
	ProblemStatus status;
	BNode* prec;
	BNode* next;

	BNode(): bi(), p_conf(), par(), status(OPEN), prec(NULL), next(NULL){}

	BNode(BranchInfo& b_info, const Configuration& d, const LagrangianResult& lr) :
	bi(b_info), p_conf(d), par(lr), status(OPEN),prec(NULL), next(NULL) {}

	~BNode(){}

	BNode(const BNode& n): bi(n.bi), p_conf(n.p_conf), par(n.par),status(n.status), 
		prec(n.prec), next(n.next){}

	void operator=(const BNode& n) {
		bi = n.bi;
		p_conf = n.p_conf;
		par = LagrangianResult(n.par);
		status = n.status;
	}
};

struct BranchParameters {
	unsigned max_time;		// default value: 0, no time limit on the computation 
};

// Classe astratta per modellare il comportamento della queue 
class IBNodeQueue {
public:
	IBNodeQueue(){}
	virtual ~IBNodeQueue() = default;
	virtual void push(BNode* node) = 0;        // Rende il metodo virtuale 
	virtual BNode* pop() = 0;                   
	virtual bool empty() const = 0;
	virtual unsigned min_lb() const = 0;
};

class DFSQueue : public IBNodeQueue{
private:
	std::stack<BNode*> stack;

public:
	~DFSQueue() {
		while (!stack.empty()) {
			BNode* n = pop();
			delete n;
		}
	}

	void push(BNode* node) override {
		stack.push(node);
	}

	BNode* pop() override {
		if (empty())
			return NULL;
		BNode* n = stack.top();
		stack.pop();
		return n;
	}

	bool empty() const override {
		return stack.empty();
	}
};

class BFSQueue : public IBNodeQueue {
private:
	std::queue<BNode*> queue;

public:
	~BFSQueue() {
		while (!queue.empty()) {
			BNode* node = pop();
			delete node;
		}
	}

	void push(BNode* node) override {
		queue.push(node);
	}

	BNode* pop() override {
		if (queue.empty()) return nullptr;
		BNode* node = queue.front();
		queue.pop();
		return node;
	}

	bool empty() const override {
		return queue.empty();
	}
};

class BestFirstQueue : public IBNodeQueue {
private:
	BNode* queue; // using a pseudo node to simplify the code. 

public:
	BestFirstQueue() {
		queue = new BNode();
		queue->bi.id = 0;
		queue->next = queue;
		queue->prec = queue;
	}

	~BestFirstQueue() {
		while (!empty()) {
			BNode* node = pop();
			delete node;
		}
		delete queue;
	}

	void push(BNode* node) override {
		BNode* p = queue->next;
		while ((node->par.lb >= p->par.lb) && p != queue)
			p = p->next;
		BNode* p_prec = p->prec;
		p_prec->next = node;
		node->prec = p_prec;
		node->next = p;
		p->prec = node;
	}

	BNode* pop() override {
		if (empty()) return nullptr;
		BNode* node = queue->next;
		queue->next = node->next;
		node->next->prec = queue;
		return node;
	}

	bool empty() const override {
		return queue->next == queue;
	}

	unsigned min_lb() const override {
		if (empty())
			return 0;
		return queue->next->par.lb;
	}

};

class BAB {
private:
	std::unique_ptr<IBNodeQueue> queue;
	LagrangianResult bounds;
	SolutionStatus status;
	BranchParameters bp;

public:
	BAB(std::unique_ptr<IBNodeQueue>&& q, LagrangianResult& lr, BranchParameters& branch_par);

	~BAB();

	unsigned branching(SetCover& ref_sc, LagrangianResult& b);

	void process_bnode(BNode* node, SetCover& sc);

	void insert_bnode(BNode* node);

	BNode* extract_bnode();

	bool useful_bnode(BNode * node, SetCover& ref_sc);

	void derive_bnode(BNode * father, BNode * son, unsigned f, const SetCover &sc);

	void derive_set_cover(SetCover& sc, Configuration &conf);

	bool problem_is_solvable(SetCover & sc, Configuration& conf);

	void update_bounds(LagrangianResult &lr);

	void branch_column(SetCover& sc, BNode* node);
};

#endif