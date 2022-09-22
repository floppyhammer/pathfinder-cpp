#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H

#include <memory>

#include "../compute_pipeline.h"
#include "program.h"

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
} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
