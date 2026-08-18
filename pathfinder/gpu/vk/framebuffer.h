#pragma once

#include <memory>

#include "../framebuffer.h"
#include "texture.h"

namespace Pathfinder {

class FramebufferVk : public Framebuffer {
    friend class DeviceVk;
    friend class SwapChainVk;

public:
    ~FramebufferVk() override;

    VkFramebuffer get_vk_handle() const;

private:
    /// Texture framebuffer.
    FramebufferVk(VkDevice vk_device, VkRenderPass vk_render_pass, const std::shared_ptr<Texture>& texture);

    /// Swap chain framebuffer.
    FramebufferVk(VkDevice vk_device, VkRenderPass vk_render_pass, Vec2I size, VkImageView vk_image_view);

    VkFramebuffer vk_framebuffer_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder
