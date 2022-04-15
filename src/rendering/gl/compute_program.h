//
// Created by floppyhammer on 6/25/2021.
//

#ifndef PATHFINDER_COMPUTE_PROGRAM_H
#define PATHFINDER_COMPUTE_PROGRAM_H

#include "program.h"
#include "data.h"
#include "../../common/global_macros.h"

namespace Pathfinder {
    class ComputeProgram : public Program {
    public:
        /// Has to use string, as vector<char> won't work.
        explicit ComputeProgram(const std::string &compute_code);

    private:
        void compile(const char *compute_code);
    };
}

#endif //PATHFINDER_COMPUTE_PROGRAM_H
