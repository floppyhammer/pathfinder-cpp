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
        /// Normal framebuffer.
        FramebufferVk(VkDevice device, VkRenderPass render_pass, std::shared_ptr<Texture> texture,
                      uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        /// Swap chain framebuffer.
        FramebufferVk(VkDevice device,
                      VkRenderPass render_pass,
                      uint32_t p_width,
                      uint32_t p_height,
                      VkImageView image_view,
                      VkSampler sampler);

        ~FramebufferVk();

        uint32_t get_unique_id() override;

        VkFramebuffer get_vk_framebuffer() const;

    private:
        VkFramebuffer vk_framebuffer;

        /// For using the color attachment of this framebuffer as a sampler.
        VkDescriptorImageInfo descriptor;

        VkDevice vk_device;
    };
}

#endif

#endif //PATHFINDER_GPU_FRAMEBUFFER_VK_H
