//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_RENDER_SYSTEM_H
#define PATHFINDER_RENDER_SYSTEM_H

#include "../system.h"
#include "../entity.h"

#include <set>

namespace Pathfinder {
    class RenderSystem : public System {
    public:
        void draw(uint32_t command_buffer);
    };
}

#endif //PATHFINDER_RENDER_SYSTEM_H
