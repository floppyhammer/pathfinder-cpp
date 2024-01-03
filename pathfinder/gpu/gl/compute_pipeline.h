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
    explicit ComputePipelineGl(const std::shared_ptr<ShaderModule> &comp_shader_module) {
        program_ = std::make_shared<ComputeProgram>(comp_shader_module);
    }

    std::shared_ptr<ComputeProgram> program_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_GL_H
