//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_RENDER_PIPELINE_H
#define PATHFINDER_RENDER_PIPELINE_H

#include "vertex_input.h"
#include "raster_program.h"
#include "pipeline.h"

#include <memory>

namespace Pathfinder {
    class RenderPipeline : public Pipeline {
    public:
        RenderPipeline(const std::string &vert_source,
                       const std::string &frag_source,
                       const std::vector<VertexInputAttributeDescription>& p_attribute_descriptions,
                       ColorBlendState p_blend_state) {
            program = std::make_shared<RasterProgram>(vert_source, frag_source);

            glGenVertexArrays(1, &vao);

            attribute_descriptions = p_attribute_descriptions;

            blend_state = p_blend_state;
        };

        ~RenderPipeline() {
            glDeleteVertexArrays(1, &vao);
        };

        inline std::shared_ptr<Program> get_program() override {
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

#endif //PATHFINDER_RENDER_PIPELINE_H
