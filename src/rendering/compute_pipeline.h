//
// Created by floppyhammer on 4/11/2022.
//

#ifndef PATHFINDER_COMPUTE_PIPELINE_H
#define PATHFINDER_COMPUTE_PIPELINE_H

#include "compute_program.h"
#include "pipeline.h"

#include <memory>

namespace Pathfinder {
    class ComputePipeline : public Pipeline {
    public:
        std::shared_ptr<ComputeProgram> program;

        inline std::shared_ptr<Program> get_program() override {
            return program;
        }
    };
}

#endif //PATHFINDER_COMPUTE_PIPELINE_H
