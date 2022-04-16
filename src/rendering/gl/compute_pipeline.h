#ifndef PATHFINDER_HAL_COMPUTE_PIPELINE_GL_H
#define PATHFINDER_HAL_COMPUTE_PIPELINE_GL_H

#include "program.h"
#include "../compute_pipeline.h"

#include <memory>

namespace Pathfinder {
    class ComputePipelineGl : public ComputePipeline {
    public:
        explicit ComputePipelineGl(const std::vector<char> &comp_source) {
            // We need to convert vector<char> to string first.
            std::string comp_source_string(comp_source.begin(), comp_source.end());

            program = std::make_shared<ComputeProgram>(comp_source_string);
        }

        explicit ComputePipelineGl(const std::string &comp_source) {
            program = std::make_shared<ComputeProgram>(comp_source);
        }

        inline std::shared_ptr<Program> get_program() {
            return program;
        }

    private:
        std::shared_ptr<ComputeProgram> program;
    };
}

#endif //PATHFINDER_HAL_COMPUTE_PIPELINE_GL_H
