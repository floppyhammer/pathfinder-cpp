#ifndef PATHFINDER_GPU_RENDER_PASS_GL_H
#define PATHFINDER_GPU_RENDER_PASS_GL_H

#include "../render_pass.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPassGl : public RenderPass {
    private:
        friend class DeviceGl;
    };
}

#endif

#endif //PATHFINDER_GPU_RENDER_PASS_GL_H
