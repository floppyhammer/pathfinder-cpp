#ifndef PATHFINDER_GPU_RENDER_PASS_GL_H
#define PATHFINDER_GPU_RENDER_PASS_GL_H

#include "../render_pass.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPassGl : public RenderPass {
        friend class DriverGl;

    public:
        explicit RenderPassGl(AttachmentLoadOp p_load_op) : load_op(p_load_op) {}

        inline AttachmentLoadOp get_attachment_load_op() const {
            return load_op;
        }

    private:
        AttachmentLoadOp load_op;
    };
}

#endif

#endif //PATHFINDER_GPU_RENDER_PASS_GL_H
