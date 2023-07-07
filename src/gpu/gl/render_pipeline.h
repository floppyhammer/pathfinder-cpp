#ifndef PATHFINDER_GPU_RENDER_PIPELINE_GL_H
#define PATHFINDER_GPU_RENDER_PIPELINE_GL_H

#include <memory>
#include <utility>

#include "../render_pipeline.h"
#include "debug_marker.h"
#include "program.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class RenderPipelineGl : public RenderPipeline {
public:
    RenderPipelineGl(const std::vector<char> &vert_source,
                     const std::vector<char> &frag_source,
                     const std::vector<VertexInputAttributeDescription> &_attribute_descriptions,
                     BlendState _blend_state,
                     std::string _label)
        : RenderPipeline(_attribute_descriptions, _blend_state, std::move(_label)) {
        program = std::make_shared<RasterProgram>(vert_source, frag_source);

        glGenVertexArrays(1, &vao);

        gl_check_error("create_render_pipeline");

        DebugMarker::label_program(program->get_id(), label + " program");
        DebugMarker::label_vao(vao, label + " VAO");
    };

    ~RenderPipelineGl() {
        glDeleteVertexArrays(1, &vao);
    };

    inline std::shared_ptr<Program> get_program() {
        return program;
    }

    inline uint32_t get_vao() const {
        return vao;
    }

private:
    std::shared_ptr<RasterProgram> program;

    uint32_t vao{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_RENDER_PIPELINE_GL_H
