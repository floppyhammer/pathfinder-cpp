//
// Created by floppyhammer on 4/2/2022.
//

#ifndef PATHFINDER_SYSTEM_H
#define PATHFINDER_SYSTEM_H

#include "entity.h"

#include <set>

namespace Pathfinder {
    /**
     * A system is any functionality that iterates upon a list of entities
     * with a certain signature of components.
     * Every system needs a list of entities, and we want some logic
     * outside of the system (in the form of a manager to maintain that list).
     * Each system can then inherit from this class which allows the System Manager
     * to keep a list of pointers to systems. Inheritance, but not virtual.
     */
    class System {
    public:
        std::set<Entity> entities;
    };
}

#endif //PATHFINDER_SYSTEM_H
