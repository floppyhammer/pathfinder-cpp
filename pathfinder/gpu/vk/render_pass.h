#pragma once

#include "../render_pass.h"
#include "base.h"

namespace Pathfinder {

class RenderPassVk : public RenderPass {
    friend class DeviceVk;

public:
    ~RenderPassVk() override;

    VkRenderPass get_vk_render_pass() const;

private:
    RenderPassVk(DeviceVk* device,
                 TextureFormat texture_format,
                 AttachmentLoadOp load_op,
                 bool is_swap_chain_pass,
                 const std::string &label);

    DeviceVk* device_{};

    VkRenderPass vk_render_pass_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder
