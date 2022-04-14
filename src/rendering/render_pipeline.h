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
        RenderPipeline() {
            glGenVertexArrays(1, &vao);
        };
        ~RenderPipeline() {
            glDeleteVertexArrays(1, &vao);
        };

        inline std::shared_ptr<Program> get_program() override {
            return program;
        }

        std::vector<VertexInputAttributeDescription> attribute_descriptions;
        std::shared_ptr<RasterProgram> program;

        unsigned int vao{};

        GLenum blend_src{};
        GLenum blend_dst{};
    };
}

#endif //PATHFINDER_RENDER_PIPELINE_H
