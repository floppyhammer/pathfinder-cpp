#include "texture.h"

#include <cassert>
#include <utility>

#include "../../common/global_macros.h"
#include "debug_marker.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

TextureVk::TextureVk(VkDevice _vk_device, const TextureDescriptor& _desc) : Texture(_desc), vk_device(_vk_device) {}

TextureVk::~TextureVk() {
    vkDestroySampler(vk_device, vk_sampler, nullptr);

    // Should be right before destroying the image itself.
    vkDestroyImageView(vk_device, vk_image_view, nullptr);

    // Destroy image handle.
    vkDestroyImage(vk_device, vk_image, nullptr);

    // Release device memory.
    vkFreeMemory(vk_device, vk_image_memory, nullptr);
}

VkImage TextureVk::get_image() const {
    return vk_image;
}

VkImageView TextureVk::get_image_view() const {
    return vk_image_view;
}

VkSampler TextureVk::get_sampler() const {
    return vk_sampler;
}

} // namespace Pathfinder

#endif
