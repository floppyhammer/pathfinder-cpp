//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_PHYSICS_SYSTEM_H
#define PATHFINDER_PHYSICS_SYSTEM_H

#include "../system.h"
#include "../entity.h"

#include <set>

namespace Pathfinder {
    class PhysicsSystem : public System {
    public:
        void update(float dt);
    };
}

#endif //PATHFINDER_PHYSICS_SYSTEM_H
