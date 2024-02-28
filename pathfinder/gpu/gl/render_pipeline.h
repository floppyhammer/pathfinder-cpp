#ifndef PATHFINDER_GPU_RENDER_PIPELINE_GL_H
#define PATHFINDER_GPU_RENDER_PIPELINE_GL_H

#include <memory>
#include <utility>

#include "../render_pipeline.h"
#include "debug_marker.h"
#include "program.h"

namespace Pathfinder {

class RenderPipelineGl : public RenderPipeline {
    friend class DeviceGl;

public:
    std::shared_ptr<Program> get_program() {
        return program_;
    }

private:
    RenderPipelineGl(const std::shared_ptr<ShaderModule> &vert_shader_module,
                     const std::shared_ptr<ShaderModule> &frag_shader_module,
                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                     BlendState blend_state,
                     std::string label)
        : RenderPipeline(attribute_descriptions, blend_state, std::move(label)) {
        program_ = std::make_shared<RasterProgram>(vert_shader_module, frag_shader_module);

        gl_check_error("create_render_pipeline");

        DebugMarker::label_program(program_->get_handle(), label_ + " program");
    }

    std::shared_ptr<RasterProgram> program_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PIPELINE_GL_H
