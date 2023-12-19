#ifndef PATHFINDER_GPU_RENDER_PASS_GL_H
#define PATHFINDER_GPU_RENDER_PASS_GL_H

#include "../render_pass.h"

namespace Pathfinder {

class RenderPassGl : public RenderPass {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    AttachmentLoadOp get_attachment_load_op() const {
        return load_op_;
    }

private:
    explicit RenderPassGl(AttachmentLoadOp load_op) : load_op_(load_op) {}

    AttachmentLoadOp load_op_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PASS_GL_H
