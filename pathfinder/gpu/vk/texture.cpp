#include "texture.h"

#include <cassert>

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

TextureVk::TextureVk(VkDevice _vk_device, const TextureDescriptor& _desc) : Texture(_desc), vk_device(_vk_device) {}

TextureVk::~TextureVk() {
    if (!resource_ownership) {
        return;
    }

    // Should be right before destroying the image itself.
    vkDestroyImageView(vk_device, vk_image_view, nullptr);

    // Destroy image handle.
    vkDestroyImage(vk_device, vk_image, nullptr);

    // Release device memory.
    vkFreeMemory(vk_device, vk_image_memory, nullptr);

    if (vk_staging_buffer) {
        vkDestroyBuffer(vk_device, vk_staging_buffer, nullptr);
        vkFreeMemory(vk_device, vk_staging_buffer_memory, nullptr);
    }
}

std::shared_ptr<TextureVk> TextureVk::from_wrapping(const TextureDescriptor& _desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    TextureLayout layout) {
    auto texture_vk = std::shared_ptr<TextureVk>(new TextureVk(nullptr, _desc));

    // We're not responsible for management of wrapped textures.
    texture_vk->resource_ownership = false;

    texture_vk->vk_image = image;
    texture_vk->vk_image_memory = image_memory;
    texture_vk->vk_image_view = image_view;
    texture_vk->layout = layout;

    return texture_vk;
}

VkImage TextureVk::get_image() const {
    return vk_image;
}

VkImageView TextureVk::get_image_view() const {
    return vk_image_view;
}

TextureLayout TextureVk::get_layout() const {
    return layout;
}

void TextureVk::set_layout(TextureLayout new_layout) {
    layout = new_layout;
}

void TextureVk::set_label(const std::string& _label) {
    if (vk_device == nullptr) {
        Logger::warn("Attempted to set label for a wrapped texture!");
        return;
    }

    Texture::set_label(_label);

    DebugMarker::get_singleton()->set_object_name(vk_device, (uint64_t)vk_image, VK_OBJECT_TYPE_IMAGE, label);
}

void TextureVk::create_staging_buffer(DeviceVk* device_vk) {
    if (vk_staging_buffer != VK_NULL_HANDLE) {
        return;
    }

    // Bytes of one pixel.
    auto pixel_size = get_pixel_size(get_format());

    uint32_t max_data_size = get_size().area() * pixel_size;

    device_vk->create_vk_buffer(max_data_size,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                vk_staging_buffer,
                                vk_staging_buffer_memory);
}

} // namespace Pathfinder
