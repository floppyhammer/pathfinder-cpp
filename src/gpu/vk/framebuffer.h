#ifndef PATHFINDER_GPU_FRAMEBUFFER_VK_H
#define PATHFINDER_GPU_FRAMEBUFFER_VK_H

#include <memory>

#include "../framebuffer.h"
#include "texture.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class FramebufferVk : public Framebuffer {
    friend class DriverVk;

public:
    /// Texture framebuffer.
    FramebufferVk(VkDevice _device, VkRenderPass render_pass, const std::shared_ptr<Texture> &_texture);

    /// Swap chain framebuffer.
    FramebufferVk(VkDevice _device, VkRenderPass render_pass, Vec2I size, VkImageView image_view);

    ~FramebufferVk();

    unsigned long long get_unique_id() override;

    VkFramebuffer get_vk_framebuffer() const;

private:
    VkFramebuffer vk_framebuffer;

    VkDevice device;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_FRAMEBUFFER_VK_H
