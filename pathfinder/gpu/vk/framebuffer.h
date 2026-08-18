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

    void set_label(const std::string& label) override;

private:
    /// Texture framebuffer.
    FramebufferVk(DeviceVk* device, VkRenderPass vk_render_pass, const std::shared_ptr<Texture>& texture);

    /// Swap chain framebuffer.
    FramebufferVk(DeviceVk* device, VkRenderPass vk_render_pass, Vec2I size, VkImageView vk_image_view);

    DeviceVk* device_{};

    VkFramebuffer vk_framebuffer_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder
