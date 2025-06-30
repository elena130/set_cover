#include <vector>
#include <queue>
#include "LagrangianData.h"
#include "status.h"
#include "setcover.h"

#ifndef BAB_H
#define BAB_H

struct DebugInfo {
	const unsigned id;
	const unsigned depht;
};

struct StructInfo {
	std::vector<Status> cols;
	std::vector<Status> rows;
};

struct Node {
	DebugInfo di;
	LagrangianResult lr;
};

class BAB {
private:
	SetCover sc;
	std::priority_queue<Node> queue;

public:
	BAB(SetCover set_cover);

	~BAB();

	void branch_and_bound();

	void process_node(Node& node);

	void insert_node(Node& node);

	Node extract_node();

	bool is_useful(Node& node);

	void derive_info(Node& father, Node& son);

	Node create_node(unsigned id, unsigned depht);
};

#endif