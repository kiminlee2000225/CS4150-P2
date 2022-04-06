#pragma once
#include "Tree.h"
#include "Selector.h"
#include "Sequence.h"

class GameAI : Tree {
public:
	Node* SetUpTree() {

		Node event1 = Node();

		std::vector<Node> children2;
		children2.push_back(event1);
		Sequence sequence1 = Sequence(children2);

		std::vector<Node> children1;
		children1.push_back(sequence1);
		Node* root = &Selector(children1);

		return root;
	}
};