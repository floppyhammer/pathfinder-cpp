#pragma once

#include <MetalKit/MetalKit.h>

#include "../render_pipeline.h"

namespace Pathfinder {

class RenderPipelineMtl final : public RenderPipeline {
    friend class DeviceMtl;

public:
    id<MTLRenderPipelineState> get_handle() {
        return pipeline_state_;
    }

    id<MTLDepthStencilState> get_depth_state() {
        return depth_stencil_state_;
    }

private:
    explicit RenderPipelineMtl(const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                               BlendState blend_state,
                               std::string label)
        : RenderPipeline(attribute_descriptions, blend_state, label) {};

    id<MTLRenderPipelineState> pipeline_state_ = nil;
    id<MTLDepthStencilState> depth_stencil_state_ = nil;
};

} // namespace Pathfinder
