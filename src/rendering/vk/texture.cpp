#include "texture.h"

#include "../../common/global_macros.h"

#include <cassert>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    TextureVk::TextureVk(VkDevice p_device, uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type)
            : Texture(p_width, p_height, p_format, p_type), device(p_device) {
    }

    TextureVk::~TextureVk() {
        vkDestroySampler(device, sampler, nullptr);

        // Should be right before destroying the image itself.
        vkDestroyImageView(device, image_view, nullptr);

        // Release GPU memory.
        vkDestroyImage(device, image, nullptr);

        // Release CPU memory.
        vkFreeMemory(device, image_memory, nullptr);
    }

    VkImage TextureVk::get_image() const {
        return image;
    }

    VkImageView TextureVk::get_image_view() const {
        return image_view;
    }

    VkSampler TextureVk::get_sampler() const {
        return sampler;
    }
}

#endif
