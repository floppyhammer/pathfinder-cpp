#include "texture.h"

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

TextureVk::TextureVk(VmaAllocator vma_allocator, VkDevice vk_device, const TextureDescriptor& desc)
    : Texture(desc), vma_allocator_(vma_allocator), vk_device_(vk_device) {}

TextureVk::~TextureVk() {
    if (!resource_ownership_) {
        return;
    }

    // Should be right before destroying the image itself.
    if (vk_device_ && vk_image_view_) {
        vkDestroyImageView(vk_device_, vk_image_view_, nullptr);
        vk_image_view_ = VK_NULL_HANDLE;
    }

    // Destroy image and free memory using VMA
    if (vma_allocator_ && vk_image_ && vma_allocation_) {
        vmaDestroyImage(vma_allocator_, vk_image_, vma_allocation_);
        vk_image_ = VK_NULL_HANDLE;
        vma_allocation_ = VK_NULL_HANDLE;
    }
}

std::shared_ptr<TextureVk> TextureVk::from_wrapping(const TextureDescriptor& desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    TextureLayout layout) {
    auto texture_vk = std::shared_ptr<TextureVk>(new TextureVk(VK_NULL_HANDLE, VK_NULL_HANDLE, desc));

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

VkDeviceMemory TextureVk::get_vk_device_memory() const {
    if (vk_image_memory_) {
        return vk_image_memory_;
    }
    if (!vma_allocator_ || !vma_allocation_) {
        return VK_NULL_HANDLE;
    }
    VmaAllocationInfo info{};
    vmaGetAllocationInfo(vma_allocator_, vma_allocation_, &info);
    return info.deviceMemory;
}

VmaAllocation TextureVk::get_vma_allocation() const {
    return vma_allocation_;
}

} // namespace Pathfinder
