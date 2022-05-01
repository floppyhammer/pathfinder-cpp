#ifndef PATHFINDER_GPU_RENDER_PASS_VK_H
#define PATHFINDER_GPU_RENDER_PASS_VK_H

#include "../render_pass.h"
#include "data.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPassVk : public RenderPass {
        friend class DriverVk;
    public:
        RenderPassVk(VkDevice device, TextureFormat texture_format, ImageLayout final_layout);

        ~RenderPassVk();

        VkRenderPass get_vk_render_pass();

    private:
        VkRenderPass vk_render_pass;

        VkDevice vk_device;
    };
}

#endif

#endif //PATHFINDER_GPU_RENDER_PASS_VK_H
