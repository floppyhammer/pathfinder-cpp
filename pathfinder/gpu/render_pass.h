#ifndef PATHFINDER_GPU_RENDER_PASS_H
#define PATHFINDER_GPU_RENDER_PASS_H

#include "../common/math/vec2.h"

namespace Pathfinder {

enum class AttachmentLoadOp;

class RenderPass {
public:
    virtual ~RenderPass() = default;

    AttachmentLoadOp get_attachment_load_op() const {
        return load_op_;
    }

protected:
    RenderPass() = default;

    AttachmentLoadOp load_op_;

    /// Debug label.
    std::string label_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PASS_H
