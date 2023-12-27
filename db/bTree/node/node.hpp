#pragma once

#include <string>
#include <vector>

enum class NodeType { NODE_ROOT, NODE_INTERNAL, NODE_LEAF };

class Node {
private:
	Node* splitLeaf(int rIndex);

	Node* splitInternal(int rIndex);

public:
	NodeType type;
	Node* parent;
	int size;

	std::vector<int> keys;

	// For only internal nodes

	std::vector<Node*> children;

	// For only leaf nodes

	std::vector<std::string> vals;
	Node* prev;
	Node* next;

	Node() : parent(nullptr), prev(nullptr), next(nullptr), size(0) {}

	~Node();

	int findKey(int key);

	int keyInsertIndex(int key);

	int indexOfChild(Node* child);

	void removeFromLeaf(int key);

	void removeFromInternal(int key);

	void borrowFromRightLeaf();	

	void borrowFromLeftLeaf();

	void mergeWithRightLeaf();

	void mergeWithLeftLeaf();

	void borrowFromRightInternal(Node* next);

	void borrowFromLeftInternal(Node* prev);

	void mergeWithRightInternal(Node* next);

	void mergeWithLeftInternal(Node* prev);

	void set(int key, std::string& val);

	Node* splitNode();
};