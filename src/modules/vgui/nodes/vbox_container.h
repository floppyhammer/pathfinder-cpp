#ifndef PATHFINDER_VBOX_CONTAINER_H
#define PATHFINDER_VBOX_CONTAINER_H

#include "container.h"

namespace Pathfinder {
    class VBoxContainer : public Container {
    public:
        void adjust_layout() override;
    };
}

#endif //PATHFINDER_VBOX_CONTAINER_H
