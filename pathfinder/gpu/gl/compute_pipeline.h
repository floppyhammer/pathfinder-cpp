#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H

#include <memory>

#include "../compute_pipeline.h"
#include "program.h"

namespace Pathfinder {

class ComputePipelineGl : public ComputePipeline {
    friend class DeviceGl;

public:
    std::shared_ptr<Program> get_program() {
        return program_;
    }

private:
    explicit ComputePipelineGl(const std::vector<char> &comp_source) {
        program_ = std::make_shared<ComputeProgram>(comp_source);
    }

    std::shared_ptr<ComputeProgram> program_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
