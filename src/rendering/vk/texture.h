#ifndef PATHFINDER_HAL_TEXTURE_VK_H
#define PATHFINDER_HAL_TEXTURE_VK_H

#include "../texture.h"
#include "../../common/math/rect.h"
#include "../../common/global_macros.h"
#include "../../common/logger.h"

#include "stb_image.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    /// Use Texture via smart pointers as its de-constructor will release its GL resources.
    class TextureVk : public Texture {
    public:
        TextureVk(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

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

        VkDevice device;
    };
}

#endif

#endif //PATHFINDER_HAL_TEXTURE_VK_H
