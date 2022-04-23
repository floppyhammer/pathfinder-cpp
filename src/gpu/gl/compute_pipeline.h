#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H

#include "program.h"
#include "../compute_pipeline.h"

#include <memory>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class ComputePipelineGl : public ComputePipeline {
    public:
        explicit ComputePipelineGl(const std::vector<char> &comp_source) {
            program = std::make_shared<ComputeProgram>(comp_source);
        }

        inline std::shared_ptr<Program> get_program() {
            return program;
        }

    private:
        std::shared_ptr<ComputeProgram> program;
    };
}

#endif

#endif //PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
