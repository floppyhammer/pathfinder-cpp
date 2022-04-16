#include "texture.h"

#include "../../common/global_macros.h"

#include <cassert>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    TextureVk::TextureVk(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type)
            : Texture(p_width, p_height, p_format, p_type) {
        RS::getSingleton().createImage(width, height,
                                       VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       image,
                                       image_memory);

        // Create image view.
        image_view = RS::getSingleton().createImageView(texture->image,
                                                        VK_FORMAT_R8G8B8A8_SRGB,
                                                        VK_IMAGE_ASPECT_COLOR_BIT);

        // Create sampler.
        RS::getSingleton().createTextureSampler(sampler);
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
