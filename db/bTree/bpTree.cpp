#include <bpTree.hpp>

BPTree::BPTree(int deg) : deg(deg) {
	this->root = new Node();
	this->root->type = NodeType::NODE_LEAF;

	return;
}

Node* BPTree::findLeaf(int key) {
	Node* curr = this->root;

	// Descend to correct leaf node

	while (curr->type != NodeType::NODE_LEAF) {
		int index = curr->keyInsertIndex(key);
		curr = curr->children[index];
	}

	return curr;
}

void BPTree::printTree() {
	const char* cPrefix = "";
	std::string prefix = cPrefix;

	this->printTree(this->root, prefix, true);

	return;
}

void BPTree::printTree(Node* node, std::string& prefix, bool last) {
	std::cout << prefix << "├ [";

	for (int i = 0; i < node->keys.size(); i++) {
		std::cout << node->keys[i];
		if (i != node->keys.size() - 1) {
			std::cout << ", ";
		}
	}

	std::cout << "]" << '\n';

	prefix += last ? "   " : "╎  ";

	if (!(node->type == NodeType::NODE_LEAF)) {
		for (int i = 0; i < node->children.size(); i++) {
			bool last = (i == node->children.size() - 1);
			printTree(node->children[i], prefix, last);
		}
	}
}

void BPTree::set(int key, std::string& val) {
	Node* curr = this->findLeaf(key);
	int index = curr->findKey(key);

	curr->set(key, val);

	if (index != -1) {
		return;
	}

	while (curr && curr->size >= deg) {
		Node* newRootNode = curr->splitNode();

		if (newRootNode) {
			this->depth++;
			this->root = newRootNode;
		}

		curr = curr->parent;
	}

	return;
}

void BPTree::remove(int key) {
	Node* curr = this->findLeaf(key);
	int index = curr->findKey(key);

	if (index != -1) {
		this->remove(key, curr);
	}

	return;
}

void BPTree::remove(int key, Node* curr) {
	int minCapacity = this->deg >> 1;

	if (curr->type == NodeType::NODE_LEAF) {
		curr->removeFromLeaf(key);
	} else {
		curr->removeFromInternal(key);
	}

	// Check if curr node is underfull as a result

	if (curr->size < minCapacity) {
		if (curr->type == NodeType::NODE_ROOT) { // If the underfull node is the root node
			if (!curr->size && !curr->children.empty()) {
				curr = curr->children[0];
				delete curr->parent;

				curr->parent = nullptr;
				this->root = curr;
				this->depth--;

				if (this->depth > 1) {
					curr->type = NodeType::NODE_ROOT;
				} else {
					curr->type = NodeType::NODE_LEAF;
				}
			}
		} else if (curr->type == NodeType::NODE_INTERNAL) { // If the underfull node is the internal node
			Node* parent = curr->parent;
			int currChildIndex = parent->indexOfChild(curr);

			Node* next = nullptr;
			Node* prev = nullptr;

			if (parent->children.size() > currChildIndex + 1) {
				next = parent->children[currChildIndex + 1];
			}

			if (currChildIndex) {
				prev = parent->children[currChildIndex - 1];
			}

			if (next && next->parent == parent && next->size > minCapacity) {
				curr->borrowFromRightInternal(next);
			} else if (prev && prev->parent == parent && prev->size > minCapacity) {
				curr->borrowFromLeftInternal(prev);
			} else if (next && next->parent == parent && next->size <= minCapacity) {
				curr->mergeWithRightInternal(next);
			} else if (prev && prev->parent == parent && prev->size <= minCapacity) {
				curr->mergeWithLeftInternal(prev);
				curr = prev;
			}
		} else { // If the underfull node is the leaf node
			Node* parent = curr->parent;
			Node* next = curr->next;
			Node* prev = curr->prev;

			if (next && next->parent == parent && next->size > minCapacity) {
				curr->borrowFromRightLeaf();
			} else if (prev && prev->parent == parent && prev->size > minCapacity) {
				curr->borrowFromLeftLeaf();
			} else if (next && next->parent == parent && next->size <= minCapacity) {
				curr->mergeWithRightLeaf();
			} else if (prev && prev->parent == parent && prev->size <= minCapacity) {
				curr->mergeWithLeftLeaf();
				curr = prev;
			}
		}
	}

	if (curr->parent) {
		this->remove(key, curr->parent);
	}

	return;
}
