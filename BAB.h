#include <vector>
#include <queue>
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

enum ProblemStatus {
	SOLVED,
	OPEN,
	UNSOLVABLE
};

struct Node {
	DebugInfo di;
	LagrangianResult lr;
	Configuration conf;
	ProblemStatus status;

	Node(const unsigned id, const unsigned depht) : di(id, depht), lr(), status(OPEN){}

	Node(const Node &n) : di(n.di), lr(n.lr), conf(n.conf), status(n.status){}

	~Node(){}

	void operator=(const Node& n) {
		di = n.di;
		lr = n.lr;
		status = n.status;
	}

	bool operator<(const Node& other) const {
		return lr.ub >= other.lr.ub;
	}
};

class BAB {
private:
	std::priority_queue<Node> queue;

public:
	BAB();

	~BAB();

	LagrangianResult branch_and_bound(SetCover& ref, const unsigned ub, Solution& ub_sol);

	void process_node(SetCover & ref, Node& node);

	void insert_node(Node& node);

	Node extract_node();

	const bool is_useful(Node& node, SetCover& sc);

	void derive_info(Node& father, Node& son);

	void create_scp_from_config(SetCover& scp, Configuration& config);

	void branch_rule();
};

#endif