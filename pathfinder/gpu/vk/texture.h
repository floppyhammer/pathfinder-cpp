#pragma once

#include "../texture.h"
#include "base.h"
#include "vma_header.h"

namespace Pathfinder {

class DeviceVk;

class TextureVk : public Texture {
    friend class DeviceVk;
    friend class CommandEncoderVk;

public:
    // Releasing GPU resources is done by itself.
    ~TextureVk() override;

    // Wrapping an external Vulkan image.
    static std::shared_ptr<TextureVk> from_wrapping(const TextureDescriptor& desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    TextureLayout layout);

    VkImage get_image() const;

    VkImageView get_image_view() const;

    TextureLayout get_layout() const;

    void set_layout(TextureLayout new_layout);

    VkDeviceMemory get_vk_device_memory() const;

    VmaAllocation get_vma_allocation() const;

private:
    TextureVk(VmaAllocator vma_allocator, VkDevice vk_device, const TextureDescriptor& desc);

    /// Handle.
    VkImage vk_image_{VK_NULL_HANDLE};

    /// VMA allocation & allocator.
    VmaAllocation vma_allocation_{VK_NULL_HANDLE};
    VmaAllocator vma_allocator_{VK_NULL_HANDLE};

    /// Device memory (for external wrapped images).
    VkDeviceMemory vk_image_memory_{VK_NULL_HANDLE};

    /// Memory size is not necessarily equal to WIDTH * HEIGHT * PIXEL_SIZE due to
    /// alignment and mipmaps.
    size_t memory_size_{};

    /// Thin wrapper over image.
    VkImageView vk_image_view_{VK_NULL_HANDLE};

    /// For releasing resources in destructor.
    /// This is null for wrapped external textures.
    VkDevice vk_device_{VK_NULL_HANDLE};

    /// The initial layout must be undefined.
    TextureLayout layout_ = TextureLayout::Undefined;
};

class SamplerVk : public Sampler {
    friend class DeviceVk;

public:
    ~SamplerVk() override {
        vkDestroySampler(vk_device_, vk_sampler_, nullptr);
    }

    VkSampler get_sampler() const {
        return vk_sampler_;
    }

private:
    SamplerVk(SamplerDescriptor descriptor, VkSampler vk_sampler, VkDevice vk_device) : Sampler(descriptor) {
        vk_sampler_ = vk_sampler;
        vk_device_ = vk_device;
    }

    VkSampler vk_sampler_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder
