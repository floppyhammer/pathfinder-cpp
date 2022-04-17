#ifndef PATHFINDER_CONTAINER_H
#define PATHFINDER_CONTAINER_H

#include "control.h"

namespace Pathfinder {
    /**
     * Containers adjust their container children's layouts automatically.
     */
    class Container : public Control {
    public:
        Container() {
            type = NodeType::Container;
        }

        virtual void adjust_layout() = 0;
    };
}

#endif //PATHFINDER_CONTAINER_H
