#ifndef PATHFINDER_GPU_RENDER_PASS_GL_H
#define PATHFINDER_GPU_RENDER_PASS_GL_H

#include "../render_pass.h"

namespace Pathfinder {
    class RenderPassGl : public RenderPass {
    private:
        friend class DeviceGl;
    };
}

#endif //PATHFINDER_GPU_RENDER_PASS_GL_H
