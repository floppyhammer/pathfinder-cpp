#include "texture.h"

#include <cassert>

#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
TextureVk::TextureVk(VkDevice p_device, uint32_t p_width, uint32_t p_height, TextureFormat p_format)
    : Texture(p_width, p_height, p_format), device(p_device) {
}

TextureVk::~TextureVk() {
    vkDestroySampler(device, sampler, nullptr);

    // Should be right before destroying the image itself.
    vkDestroyImageView(device, image_view, nullptr);

    // Destroy image handle.
    vkDestroyImage(device, image, nullptr);

    // Release device memory.
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
} // namespace Pathfinder

#endif
