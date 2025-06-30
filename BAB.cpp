#include "BAB.h"

BAB::BAB(SetCover set_cover) : sc(set_cover) {}

BAB::~BAB(){}

void BAB::branch_and_bound(){
	unsigned id = 0;
	Node root;
	root.di.id = 1;
	root.di.depht = 0;

	process_node(root);
	insert_node(root);

	while (!queue.empty()) {
		Node father = extract_node();

		if (is_useful(father)) {
			for (unsigned i = 0; i < 2; i++) {
				Node son = create_node(++id, father.di.depht + 1);
				derive_info(father, son);
				process_node(son);

				if (is_useful(son)) {
					insert_node(son);
				}
			}
		}
	}

}

void BAB::process_node(Node& node){

}

void BAB::insert_node(Node& node){
	queue.push(node);
}

Node BAB::extract_node() {
	Node n = queue.top();
	queue.pop();
	return n;
}

bool BAB::is_useful(Node& node){
	return false;
}

void BAB::derive_info(Node& father, Node& son){
	son.lr = father.lr;
}

Node BAB::create_node(unsigned id, unsigned depht){
	Node n;
	n.di.id = id;
	n.di.depht = depht;

	return n;
}


