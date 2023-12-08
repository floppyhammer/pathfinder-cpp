#include "texture.h"

#include <cassert>

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

TextureVk::TextureVk(VkDevice vk_device, const TextureDescriptor& desc) : Texture(desc), vk_device_(vk_device) {}

TextureVk::~TextureVk() {
    // The staging buffer is not involved in external wrapping,
    // so it always needs to be released.
    if (vk_staging_buffer_) {
        vkDestroyBuffer(vk_device_, vk_staging_buffer_, nullptr);
        vkFreeMemory(vk_device_, vk_staging_buffer_memory_, nullptr);
    }

    if (!resource_ownership_) {
        return;
    }

    // Should be right before destroying the image itself.
    vkDestroyImageView(vk_device_, vk_image_view_, nullptr);

    // Destroy image handle.
    vkDestroyImage(vk_device_, vk_image_, nullptr);

    // Release device memory.
    vkFreeMemory(vk_device_, vk_image_memory_, nullptr);
}

std::shared_ptr<TextureVk> TextureVk::from_wrapping(const TextureDescriptor& desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    TextureLayout layout) {
    auto texture_vk = std::shared_ptr<TextureVk>(new TextureVk(nullptr, desc));

    // We're not responsible for management of wrapped textures.
    texture_vk->resource_ownership_ = false;

    texture_vk->vk_image_ = image;
    texture_vk->vk_image_memory_ = image_memory;
    texture_vk->vk_image_view_ = image_view;
    texture_vk->layout_ = layout;

    return texture_vk;
}

VkImage TextureVk::get_image() const {
    return vk_image_;
}

VkImageView TextureVk::get_image_view() const {
    return vk_image_view_;
}

TextureLayout TextureVk::get_layout() const {
    return layout_;
}

void TextureVk::set_layout(TextureLayout new_layout) {
    layout_ = new_layout;
}

void TextureVk::set_label(const std::string& _label) {
    if (vk_device_ == nullptr) {
        Logger::warn("Attempted to set label for a wrapped texture!");
        return;
    }

    Texture::set_label(_label);

    DebugMarker::get_singleton()->set_object_name(vk_device_, (uint64_t)vk_image_, VK_OBJECT_TYPE_IMAGE, label_);
}

void TextureVk::create_staging_buffer(DeviceVk* device_vk) {
    if (vk_staging_buffer_ != VK_NULL_HANDLE) {
        return;
    }

    // Bytes of one pixel.
    auto pixel_size = get_pixel_size(get_format());

    uint32_t max_data_size = get_size().area() * pixel_size;

    device_vk->create_vk_buffer(max_data_size,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                vk_staging_buffer_,
                                vk_staging_buffer_memory_);
}

} // namespace Pathfinder
