#ifndef PATHFINDER_GPU_RENDER_PASS_VK_H
#define PATHFINDER_GPU_RENDER_PASS_VK_H

#include "../render_pass.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPassVk : public RenderPass {
        friend class DriverVk;
    public:
        inline VkRenderPass get_render_pass() {
            return id;
        }

    private:
        VkRenderPass id;
    };
}

#endif

#endif //PATHFINDER_GPU_RENDER_PASS_VK_H
