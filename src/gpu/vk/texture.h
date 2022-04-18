#ifndef PATHFINDER_GPU_TEXTURE_VK_H
#define PATHFINDER_GPU_TEXTURE_VK_H

#include "../texture.h"
#include "../../common/math/rect.h"
#include "../../common/global_macros.h"
#include "../../common/logger.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class TextureVk : public Texture {
    public:
        TextureVk(VkDevice p_device, uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        ~TextureVk();

        VkImage get_image() const;

        VkImageView get_image_view() const;

        VkSampler get_sampler() const;

    private:
        /// GPU memory.
        VkImage image{};

        /// CPU memory.
        VkDeviceMemory image_memory{};

        /// Thin wrapper.
        VkImageView image_view{};

        /// How image should be filtered.
        VkSampler sampler{};

        /// For releasing resources in destructor.
        VkDevice device;

        friend class DeviceVk;
    };
}

#endif

#endif //PATHFINDER_GPU_TEXTURE_VK_H
