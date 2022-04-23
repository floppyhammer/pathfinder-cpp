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
        /// To screen viewport.
        FramebufferVk(int p_width, int p_height);

        /// To texture.
        FramebufferVk(int p_width, int p_height, TextureFormat p_format, DataType p_type);

        ~FramebufferVk();

        VkFramebuffer get_framebuffer_id() const;

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
    };
}

#endif

#endif //PATHFINDER_GPU_FRAMEBUFFER_VK_H
