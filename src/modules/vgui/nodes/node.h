#ifndef PATHFINDER_NODE_H
#define PATHFINDER_NODE_H

namespace Pathfinder {
    enum class NodeType {
        Node = 0,
        Control,
        TextureRect,

        Container,
        HBoxContainer,
        VBoxContainer,

        Label,
        SubViewportContainer,

        Max,
    };

    class Node {
    public:
        Node();

        NodeType get_type() const;

    protected:
        NodeType type;
    };
}

#endif //PATHFINDER_NODE_H
