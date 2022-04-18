#ifndef PATHFINDER_HAL_FRAMEBUFFER_VK_H
#define PATHFINDER_HAL_FRAMEBUFFER_VK_H

#include "texture.h"
#include "../framebuffer.h"

#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class FramebufferVk : public Framebuffer {
    public:
        /// To screen viewport.
        FramebufferVk(int p_width, int p_height);

        /// To texture.
        FramebufferVk(int p_width, int p_height, TextureFormat p_format, DataType p_type);

        ~FramebufferVk();

        uint32_t get_framebuffer_id() const;

        std::shared_ptr<Texture> get_texture() override;

        uint32_t get_unique_id() override;

    private:
        VkFramebuffer framebuffer_id;

        VkDescriptorImageInfo descriptor;

        // Only valid when drawing to a texture.
        std::shared_ptr<TextureVk> texture;

        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        friend class DeviceVk;
    };
}

#endif

#endif //PATHFINDER_HAL_FRAMEBUFFER_VK_H
