#include "texture.h"

#include <cassert>
#include <utility>

#include "../../common/global_macros.h"
#include "debug_marker.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

TextureVk::TextureVk(VkDevice _vk_device, const TextureDescriptor& _desc) : Texture(_desc), vk_device(_vk_device) {}

TextureVk::~TextureVk() {
    if (!resource_ownership) {
        return;
    }

    vkDestroySampler(vk_device, vk_sampler, nullptr);

    // Should be right before destroying the image itself.
    vkDestroyImageView(vk_device, vk_image_view, nullptr);

    // Destroy image handle.
    vkDestroyImage(vk_device, vk_image, nullptr);

    // Release device memory.
    vkFreeMemory(vk_device, vk_image_memory, nullptr);
}

std::shared_ptr<TextureVk> TextureVk::from_wrapping(const TextureDescriptor& _desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    VkSampler sampler,
                                                    TextureLayout layout) {
    auto texture_vk = std::make_shared<TextureVk>(VK_NULL_HANDLE, _desc);

    texture_vk->resource_ownership = false;
    texture_vk->vk_image = image;
    texture_vk->vk_image_memory = image_memory;
    texture_vk->vk_image_view = image_view;
    texture_vk->vk_sampler = sampler;
    texture_vk->layout = layout;

    return texture_vk;
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
