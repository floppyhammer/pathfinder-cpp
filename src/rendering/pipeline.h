//
// Created by floppyhammer on 4/11/2022.
//

#ifndef PATHFINDER_PIPELINE_H
#define PATHFINDER_PIPELINE_H

#include "program.h"

namespace Pathfinder {
    class Pipeline {
    public:
        std::string name;

        virtual std::shared_ptr<Program> get_program() = 0;
    };
}

#endif //PATHFINDER_PIPELINE_H
