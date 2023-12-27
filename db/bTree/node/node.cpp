#include "node.hpp"

// * Node stuff (prolly move it over 'cuz it'll b the pages)

Node::~Node() {
	this->keys.clear();
	this->children.clear();
	this->vals.clear();
	this->prev = nullptr;
	this->next = nullptr;
	this->parent = nullptr;

	return;
}

Node* Node::splitLeaf(int rIndex) {
	Node* newSiblingNode = new Node();

	// Set up the new sibling node

	newSiblingNode->type = NodeType::NODE_LEAF;

	// Update leaf nodes linked list

	newSiblingNode->prev = this; // Left pointer of the new node
	newSiblingNode->next = this->next; // Right pointer of the new node
	this->next = newSiblingNode; // Make the original node point to the new node

	if (newSiblingNode->next) { // Check if the original node pointed to another node to its right
		newSiblingNode->next->prev = newSiblingNode; // Make that node point to the new node
	}

	// Transfer keys and vals over to new leaf node

	for (int i = this->size - 1; i >= rIndex; i--) {
		int key = this->keys[i];
		std::string val = this->vals[i];

		newSiblingNode->set(key, val);
		this->keys.pop_back();
		this->vals.pop_back();
	}

	// Cleanup & set sizes of the current and new nodes

	this->keys.shrink_to_fit();
	this->children.shrink_to_fit();
	this->vals.shrink_to_fit();

	newSiblingNode->size = newSiblingNode->keys.size();
	this->size = this->keys.size();

	return newSiblingNode;
}

Node* Node::splitInternal(int rIndex) {
	Node* newSiblingNode = new Node();

	newSiblingNode->type = NodeType::NODE_INTERNAL;
	newSiblingNode->keys.resize(this->size - rIndex - 1);
	newSiblingNode->children.resize(this->size - rIndex);

	// Transfer children over to new internal node

	for (int i = this->size; i > rIndex; i--) {
		newSiblingNode->children[i - rIndex - 1] = this->children[i];
		this->children[i]->parent = newSiblingNode; // Update the parents of child nodes being split off to the new sibling node
		this->children.pop_back();
	}

	// Transfer keys over to the new node

	newSiblingNode->size = newSiblingNode->keys.size();

	for (int i = this->size - 1; i >= this->size - newSiblingNode->size; i--) {
		newSiblingNode->keys[i - (this->size - newSiblingNode->size)] = this->keys[i];
		this->keys.pop_back();
	}

	this->keys.pop_back(); // Since non-leaf nodes don't keep the key being lifted to the parent node
	this->keys.shrink_to_fit();
	this->children.shrink_to_fit();
	this->vals.shrink_to_fit();

	this->size = this->keys.size();

	return newSiblingNode;
}

int Node::findKey(int key) {
	int l = 0;
	int r = this->keys.size() - 1;

	while (l <= r) {
		int mid = (l + r) >> 1;

		if (this->keys[mid] == key) {
			return mid;
		} else if (this->keys[mid] < key) {
			l = mid + 1;
		} else {
			r = mid - 1;
		}
	}

	return -1;
}

int Node::keyInsertIndex(int key) {
	return upper_bound(this->keys.begin(), this->keys.end(), key) - this->keys.begin();
}

int Node::indexOfChild(Node* child) {
	// Find the index in the children std::vector of a child node

	for (int i = 0; i < this->children.size(); i++) {
		if (this->children[i] == child) {
			return i;
		}
	}

	return -1;
}

void Node::removeFromLeaf(int key) {
	int index = this->findKey(key);

	if (index == -1) {
		return;
	}

	this->keys.erase(this->keys.begin() + index);
	this->vals.erase(this->vals.begin() + index);

	this->size--;

	if (this->parent) {
		// TODO possible error point

		int cIndex = this->parent->findKey(key); // The index of the current leaf node in the parent node

		if (cIndex == -1) {
			return;
		}

		this->parent->keys[cIndex] = this->keys.front(); // Update the key in the parent node to match the new lowest key in the current leaf node
	}

	return;
}

void Node::removeFromInternal(int key) {
	int index = this->findKey(key);

	if (index == -1) {
		return;
	}

	// TODO idk if this works

	Node* leftMostLeaf = this->children[index];

	while (leftMostLeaf->type != NodeType::NODE_LEAF) {
		leftMostLeaf = leftMostLeaf->children.front();
	}

	this->keys[index] = leftMostLeaf->keys.front();
	this->size--;

	return;
}

void Node::borrowFromRightLeaf() {
	Node* next = this->next;
	Node* nextParent = next->parent;

	// Transfer over key

	this->keys.push_back(next->keys.front());
	this->vals.push_back(next->vals.front());
	next->keys.erase(next->keys.begin());
	next->vals.erase(next->vals.begin());

	this->size++;
	next->size--;

	// Update the key in the parent node to the inorder successor (since the inorder predecessor's smallest key doesn't change)

	// TODO possible point of error - what if indexOfChild was 0...?

	nextParent->keys[nextParent->indexOfChild(next) - 1] = next->keys.front();

	return;
}

void Node::borrowFromLeftLeaf() {
	Node* prev = this->prev;
	Node* parent = this->parent;

	// Transfer over key

	this->keys.insert(this->keys.begin(), prev->keys.back());
	this->vals.insert(this->vals.begin(), prev->vals.back());
	prev->keys.pop_back();
	prev->vals.pop_back();

	this->size++;
	prev->size--;

	// Update the key in the parent node to the new inorder successor

	parent->keys[parent->indexOfChild(this) - 1] = this->keys.front();

	return;
}

void Node::mergeWithRightLeaf() {
	Node* next = this->next;
	Node* nextParent = next->parent;

	// Merge the current node with the next node (transfer keys and values over into the current node)

	for (int i = 0; i < next->size; i++) {
		int key = next->keys[i];
		std::string val = next->vals[i];

		this->set(key, val);
	}

	// Pointer management

	this->next = next->next;

	if (this->next) {
		this->next->prev = this;
	}

	// Update keys of the next node's parent node

	int childIndex = nextParent->indexOfChild(next);

	nextParent->keys.erase(nextParent->keys.begin() + childIndex - 1);
	nextParent->children.erase(nextParent->children.begin() + childIndex);

	nextParent->size--;

	// Delete the now-unused next node

	delete next;

	return;
}

void Node::mergeWithLeftLeaf() {
	Node* prev = this->prev;
	Node* parent = this->parent;

	// Merge the current node with the prev node (transfer keys and values over into the previous node)

	for (int i = 0; i < this->size; i++) {
		int key = this->keys[i];
		std::string val = this->vals[i];

		prev->set(key, val);
	}

	// Pointer management

	prev->next = this->next;

	if (prev->next) {
		prev->next->prev = prev;
	}

	// Update keys of the current node's parent node

	int childIndex = parent->indexOfChild(this);

	parent->keys.erase(parent->keys.begin() + childIndex - 1);
	parent->children.erase(parent->children.begin() + childIndex);

	parent->size--;

	// Delete the now-unused current node

	delete this;

	return;
}

void Node::borrowFromRightInternal(Node* next) {
	Node* parent = this->parent;
	int childIndex = parent->indexOfChild(this);

	// Transfer the key over to the current node

	this->keys.push_back(parent->keys[childIndex]);
	parent->keys[childIndex] = next->keys.front();
	next->keys.erase(this->keys.begin());

	this->size++;
	next->size--;

	// Update pointers

	this->children.push_back(next->children.front());
	next->children.erase(next->children.begin());
	this->children.back()->parent = this;

	return;
}

void Node::borrowFromLeftInternal(Node* prev) {
	Node* parent = this->parent;
	int childIndex = parent->indexOfChild(this);

	// Transfer the key over to the current node

	this->keys.insert(this->keys.begin(), parent->keys[childIndex - 1]);
	parent->keys[childIndex - 1] = prev->keys.back();
	prev->keys.pop_back();

	this->size++;
	prev->size--;

	// Update pointers

	this->children.insert(this->children.begin(), prev->children.back());
	prev->children.pop_back();
	this->children.front()->parent = this;

	return;
}

void Node::mergeWithRightInternal(Node* next) {
	Node* parent = this->parent;
	int childIndex = parent->indexOfChild(this);

	// Transfer key from the parent node to the current node

	this->keys.push_back(parent->keys[childIndex]);
	parent->keys.erase(parent->keys.begin() + childIndex);
	parent->children.erase(parent->children.begin() + childIndex + 1); // Delete the next node from the parent

	this->size += next->size + 1; // All keys from the next node + the key from the parent
	parent->size--;

	// Merge the current node with the next node (transfer keys and children over into the current node)

	// TODO maybe something wrong here

	for (int key : next->keys) {
		this->keys.push_back(key);
	}

	for (Node* child : next->children) {
		this->children.push_back(child);
		child->parent = this;
	}

	// Delete the now-unused next node

	delete next;

	return;
}

void Node::mergeWithLeftInternal(Node* prev) {
	Node* parent = this->parent;
	int childIndex = parent->indexOfChild(this);

	// Transfer key from the parent node to the prev node

	prev->keys.push_back(parent->keys[childIndex - 1]);
	parent->keys.erase(parent->keys.begin() + childIndex - 1);
	parent->children.erase(parent->children.begin() + childIndex);

	prev->size += this->size + 1;
	parent->size--;

	// Merge the current node with the prev node (transfer keys and children over into the prev node)

	// TODO maybe something wrong here

	for (int key : this->keys) {
		prev->keys.push_back(key);
	}

	for (Node* child : this->children) {
		prev->children.push_back(child);
		child->parent = prev;
	}

	// Delete the now-unused curr node

	delete this;

	return;
}

void Node::set(int key, std::string& val) {
	int keyIndex = this->findKey(key);

	if (keyIndex != -1) {
		this->vals[keyIndex] = val;

		return;
	}

	Node* curr = this;
	int insIndex = curr->keyInsertIndex(key);

	curr->keys.insert(curr->keys.begin() + insIndex, key);
	curr->vals.insert(curr->vals.begin() + insIndex, val);
	curr->size++;

	return;
}

Node* Node::splitNode() {
	int rIndex = this->size >> 1; // Right split start index: [rIndex, Node->size]; Left split end index: [0, rIndex)
	int newParentKey = this->keys[rIndex];

	Node* siblingNode;

	if (this->type == NodeType::NODE_LEAF) {
		siblingNode = this->splitLeaf(rIndex);
	} else {
		siblingNode = this->splitInternal(rIndex);
	}

	// Make changes to the parent node

	if (this->parent) {
		Node* parentNode = this->parent;

		int index = parentNode->keyInsertIndex(newParentKey);
		parentNode->keys.insert(parentNode->keys.begin() + index, newParentKey);
		parentNode->size++;

		if (index > 0) {
			parentNode->children.insert(parentNode->children.begin() + index + 1, siblingNode);
		} else {
			parentNode->children.insert(parentNode->children.begin(), this);
			parentNode->children.insert(parentNode->children.begin() + 1, siblingNode);
		}

		siblingNode->parent = parentNode;
	} else {
		Node* newRootNode = new Node();

		newRootNode->type = NodeType::NODE_ROOT;
		newRootNode->keys.push_back(newParentKey);
		newRootNode->size = 1;
		newRootNode->children.push_back(this);
		newRootNode->children.push_back(siblingNode);

		// Update the nodes at the original level

		if (this->type == NodeType::NODE_ROOT) {
			this->type = NodeType::NODE_INTERNAL;
		}

		this->parent = newRootNode;
		siblingNode->parent = newRootNode;

		return newRootNode;
	}

	return nullptr;
}