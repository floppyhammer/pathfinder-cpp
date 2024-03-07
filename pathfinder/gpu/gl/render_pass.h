#ifndef PATHFINDER_GPU_RENDER_PASS_GL_H
#define PATHFINDER_GPU_RENDER_PASS_GL_H

#include "../render_pass.h"

namespace Pathfinder {

class RenderPassGl : public RenderPass {
    friend class DeviceGl;
    friend class SwapChainGl;

private:
    explicit RenderPassGl(AttachmentLoadOp load_op) : load_op_(load_op) {}
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PASS_GL_H
