#ifndef PATHFINDER_GPU_FRAMEBUFFER_VK_H
#define PATHFINDER_GPU_FRAMEBUFFER_VK_H

#include "texture.h"
#include "../framebuffer.h"

#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class FramebufferVk : public Framebuffer {
        friend class DriverVk;

    public:
        /// Texture framebuffer.
        FramebufferVk(VkDevice p_device, VkRenderPass render_pass, const std::shared_ptr<Texture> &p_texture);

        /// Swap chain framebuffer.
        FramebufferVk(VkDevice p_device,
                      VkRenderPass render_pass,
                      uint32_t p_width,
                      uint32_t p_height,
                      VkImageView image_view);

        ~FramebufferVk();

        unsigned long long get_unique_id() override;

        VkFramebuffer get_vk_framebuffer() const;

    private:
        VkFramebuffer vk_framebuffer;

        /// For using the color attachment of this framebuffer as a sampler.
        VkDescriptorImageInfo descriptor;

        VkDevice device;
    };
}

#endif

#endif //PATHFINDER_GPU_FRAMEBUFFER_VK_H
