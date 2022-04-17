#include "node.h"

namespace Pathfinder {
    Node::Node() {
        type = NodeType::Node;
    }

    NodeType Node::get_type() const {
        return type;
    }
}
