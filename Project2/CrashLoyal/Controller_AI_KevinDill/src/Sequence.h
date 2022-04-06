#pragma once
#include "Node.h"

class Sequence : public Node {
public:
	Sequence() : Node() {};
	Sequence(const std::vector<Node> &children) : Node(children) {};

	NodeState Evaluate() {
		bool anyChildIsRunning = false;
		for (Node node : children) {
			switch (node.Evaluate()) {
				case NodeState::FAILURE:
					state = NodeState::FAILURE;
					return state;
				case NodeState::RUNNING:
					anyChildIsRunning = true;
					continue;
				case NodeState::SUCCESS:
					continue;
				default:
					state = NodeState::SUCCESS;
					return state;
			}
		}
		state = anyChildIsRunning ? NodeState::RUNNING : NodeState::SUCCESS;
		return state;
	}
};