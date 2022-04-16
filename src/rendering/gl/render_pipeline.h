#ifndef PATHFINDER_RENDER_PIPELINE_GL_H
#define PATHFINDER_RENDER_PIPELINE_GL_H

#include "program.h"
#include "../vertex_input.h"
#include "../render_pipeline.h"

#include <memory>

namespace Pathfinder {
    class RenderPipelineGl : public RenderPipeline {
    public:
        RenderPipelineGl(const std::string &vert_source,
                         const std::string &frag_source,
                         const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
                         ColorBlendState p_blend_state) {
            program = std::make_shared<RasterProgram>(vert_source, frag_source);

            glGenVertexArrays(1, &vao);

            attribute_descriptions = p_attribute_descriptions;

            blend_state = p_blend_state;
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

        inline const std::vector<VertexInputAttributeDescription> &get_attribute_descriptions() const {
            return attribute_descriptions;
        }

        inline ColorBlendState get_blend_state() const {
            return blend_state;
        }

    private:
        std::shared_ptr<RasterProgram> program;

        uint32_t vao{};

        std::vector<VertexInputAttributeDescription> attribute_descriptions;

        ColorBlendState blend_state{};
    };
}

#endif //PATHFINDER_RENDER_PIPELINE_GL_H
