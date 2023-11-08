#ifndef PATHFINDER_GPU_RENDER_PASS_VK_H
#define PATHFINDER_GPU_RENDER_PASS_VK_H

#include "../render_pass.h"
#include "base.h"

namespace Pathfinder {

class RenderPassVk : public RenderPass {
    friend class DeviceVk;

public:
    ~RenderPassVk() override;

    VkRenderPass get_vk_render_pass();

private:
    RenderPassVk(VkDevice _device,
                 TextureFormat texture_format,
                 AttachmentLoadOp load_op,
                 bool is_swap_chain_pass,
                 const std::string &label);

private:
    VkRenderPass vk_render_pass{};

    VkDevice vk_device{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PASS_VK_H
