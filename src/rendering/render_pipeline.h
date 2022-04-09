//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_RENDER_PIPELINE_H
#define PATHFINDER_RENDER_PIPELINE_H

#include "vertex_input.h"
#include "raster_program.h"

#include <memory>

namespace Pathfinder {
    class RenderPipeline {
    public:
        std::shared_ptr<RasterProgram> program;
        VertexInputState vertex_input_state;
        std::vector<AttributeDescriptor> attribute_descriptors;

        GLenum blend_src{};
        GLenum blend_dst{};
    };
}

#endif //PATHFINDER_RENDER_PIPELINE_H
