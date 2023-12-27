#pragma once

#include "node/node.hpp"
#include <iostream>

class BPTree {
private:
	Node* findLeaf(int key);

public:
	int deg;
	int depth = 1;
	Node* root;

	BPTree(int deg);

	void printTree();

	void printTree(Node* node, std::string& prefix, bool last);

	void set(int key, std::string& val);

	void remove(int key);

	void remove(int key, Node* curr);
};