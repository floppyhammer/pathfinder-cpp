#ifndef PATHFINDER_GPU_RENDER_PIPELINE_H
#define PATHFINDER_GPU_RENDER_PIPELINE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base.h"

namespace Pathfinder {

class RenderPipeline {
public:
    RenderPipeline(const std::vector<VertexInputAttributeDescription>& attribute_descriptions,
                   BlendState blend_state,
                   std::string label)
        : attribute_descriptions_(attribute_descriptions), blend_state_(blend_state), label_(std::move(label)){};

    virtual ~RenderPipeline() = default;

    const std::vector<VertexInputAttributeDescription>& get_attribute_descriptions() const {
        return attribute_descriptions_;
    }

    BlendState get_blend_state() const {
        return blend_state_;
    }

protected:
    std::vector<VertexInputAttributeDescription> attribute_descriptions_;

    BlendState blend_state_{};

    /// Debug label.
    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PIPELINE_H
