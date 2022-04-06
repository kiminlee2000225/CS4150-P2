#pragma once
#include "Node.h"

class Tree {
public:

protected:
	void Start() { _root = SetUpTree(); };
	Node SetUpTree();

private:
	Node* _root = nullptr;

	void Update() {
		if (_root != nullptr) {
			_root->Evaluate();
		}
	}
};