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
    RenderPipeline(const std::vector<VertexInputAttributeDescription>& _attribute_descriptions,
                   BlendState _blend_state,
                   std::string _label)
        : attribute_descriptions(_attribute_descriptions), blend_state(_blend_state), label(std::move(_label)){};

    virtual ~RenderPipeline() = default;

    inline const std::vector<VertexInputAttributeDescription>& get_attribute_descriptions() const {
        return attribute_descriptions;
    }

    inline BlendState get_blend_state() const {
        return blend_state;
    }

protected:
    std::vector<VertexInputAttributeDescription> attribute_descriptions;

    BlendState blend_state{};

    /// Debug label.
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PIPELINE_H
