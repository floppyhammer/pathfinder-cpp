#ifndef PATHFINDER_GPU_RENDER_PASS_H
#define PATHFINDER_GPU_RENDER_PASS_H

#include "../common/global_macros.h"
#include "../common/math/vec2.h"

namespace Pathfinder {

class RenderPass {
public:
    virtual ~RenderPass() = default;

protected:
    RenderPass() = default;

protected:
    /// Debug label.
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PASS_H
