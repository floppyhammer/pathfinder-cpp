#include "texture.h"

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

TextureVk::TextureVk(VkDevice vk_device, const TextureDescriptor& desc) : Texture(desc), vk_device_(vk_device) {}

TextureVk::~TextureVk() {
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

void TextureVk::set_label(const std::string& label) {
    if (vk_device_ == nullptr) {
        Logger::warn("Attempted to set label for a wrapped texture!");
        return;
    }

    Texture::set_label(label);

    DebugMarker::get_singleton()->set_object_name(vk_device_, (uint64_t)vk_image_, VK_OBJECT_TYPE_IMAGE, label);
}

} // namespace Pathfinder
