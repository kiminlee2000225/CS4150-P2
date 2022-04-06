#pragma once
#include <vector>
#include <map>
#include <string>

enum NodeState {
    RUNNING,
    SUCCESS,
    FAILURE,
};

class Node {
public:
    Node *parent;

    Node() {
        parent = nullptr;
    };

    Node(std::vector<Node> children) {
        for (Node child : children) {
            _Attach(child);
        }
    };

    ~Node();

    NodeState Evaluate() { return NodeState::FAILURE; };

    void SetData(std::string key, std::string value) {
        _dataContext[key] = value;
    }

    std::string* GetData(std::string key) {
        if (_dataContext.find(key) != _dataContext.end()) {
            return &_dataContext[key];
        }

       // std::string value = nullptr;
        Node* node = parent;
        while (node != nullptr) {
            if (node->GetData(key) != nullptr) {
                return node->GetData(key);
            }
            node = node->parent;
        }
        return nullptr;
    }

    bool ClearData(std::string key) {
        if (_dataContext.find(key) != _dataContext.end()) {
            _dataContext.erase(_dataContext.find(key));
            return true;
        }

       // std::string value = nullptr;
        Node* node = parent;
        while (node != nullptr) {
            bool cleared = node->ClearData(key);
            if (cleared) {
                return true;
            }
            node = node->parent;
        }
        return false;
    }

protected:
    NodeState state;
    std::vector<Node> children;

private:
    void _Attach(Node node) {
        node.parent = this;
        children.push_back(node);
    }

    std::map<std::string, std::string> _dataContext = std::map<std::string, std::string>();
};