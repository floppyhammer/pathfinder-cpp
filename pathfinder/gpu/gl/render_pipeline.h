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
    ~RenderPipelineGl() override {
        glDeleteVertexArrays(1, &vao_);
    };

    std::shared_ptr<Program> get_program() {
        return program_;
    }

    uint32_t get_vao() const {
        return vao_;
    }

private:
    RenderPipelineGl(const std::vector<char> &vert_source,
                     const std::vector<char> &frag_source,
                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                     BlendState blend_state,
                     std::string label)
        : RenderPipeline(attribute_descriptions, blend_state, std::move(label)) {
        program_ = std::make_shared<RasterProgram>(vert_source, frag_source);

        glGenVertexArrays(1, &vao_);

        gl_check_error("create_render_pipeline");

        DebugMarker::label_program(program_->get_id(), label_ + " program");
        DebugMarker::label_vao(vao_, label_ + " VAO");
    };

    std::shared_ptr<RasterProgram> program_;

    uint32_t vao_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PIPELINE_GL_H
