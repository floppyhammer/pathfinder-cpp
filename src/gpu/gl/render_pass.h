#ifndef PATHFINDER_GPU_RENDER_PASS_GL_H
#define PATHFINDER_GPU_RENDER_PASS_GL_H

#include "../render_pass.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class RenderPassGl : public RenderPass {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    inline AttachmentLoadOp get_attachment_load_op() const {
        return load_op;
    }

private:
    explicit RenderPassGl(AttachmentLoadOp _load_op) : load_op(_load_op) {}

private:
    AttachmentLoadOp load_op;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_RENDER_PASS_GL_H
