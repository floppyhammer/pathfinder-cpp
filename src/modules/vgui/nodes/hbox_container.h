#ifndef PATHFINDER_HBOX_CONTAINER_H
#define PATHFINDER_HBOX_CONTAINER_H

#include "container.h"

namespace Pathfinder {
    class HBoxContainer : public Container {
    public:
        HBoxContainer() {
            type = NodeType::HBoxContainer;
        }

        void adjust_layout() override;
    };
}

#endif //PATHFINDER_HBOX_CONTAINER_H
