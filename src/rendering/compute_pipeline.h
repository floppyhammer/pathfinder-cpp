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
        explicit ComputePipeline(const std::vector<char> &comp_source) {
            // We need to convert vector<char> to string first.
            std::string comp_source_string(comp_source.begin(), comp_source.end());

            program = std::make_shared<ComputeProgram>(comp_source_string);
        }

        explicit ComputePipeline(const std::string &comp_source) {
            program = std::make_shared<ComputeProgram>(comp_source);
        }

        inline std::shared_ptr<Program> get_program() override {
            return program;
        }

    private:
        std::shared_ptr<ComputeProgram> program;
    };
}

#endif //PATHFINDER_COMPUTE_PIPELINE_H
