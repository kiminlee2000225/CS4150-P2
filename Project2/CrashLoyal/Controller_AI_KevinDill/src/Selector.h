#pragma once
#include "Node.h"

class Selector : public Node {
public:
	Selector() : Node() {};
	Selector(const std::vector<Node> &children) : Node(children) {};

	NodeState Evaluate() {
		for (Node node : children) {
			switch (node.Evaluate()) {
			case NodeState::FAILURE:
				continue;
			case NodeState::RUNNING:
				state = NodeState::RUNNING;
				return state;
			case NodeState::SUCCESS:
				state = NodeState::SUCCESS;
				return state;
			default:
				continue;
			}
		}
		state = NodeState::FAILURE;
		return state;
	}
};