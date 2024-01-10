#ifndef PATHFINDER_GPU_TEXTURE_VK_H
#define PATHFINDER_GPU_TEXTURE_VK_H

#include "../texture.h"
#include "base.h"

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

    void set_label(const std::string& label) override;

private:
    // This constructor is only a wrapper, actual GPU resource allocation is done by DeviceVk.
    TextureVk(VkDevice vk_device, const TextureDescriptor& desc);

    void create_staging_buffer(DeviceVk* device_vk);

    /// Handle.
    VkImage vk_image_{};

    /// Device memory.
    VkDeviceMemory vk_image_memory_{};

    /// Memory size is not necessarily equal to WIDTH * HEIGHT * PIXEL_SIZE due to
    /// alignment and mipmaps.
    size_t memory_size_{};

    /// Thin wrapper over image.
    VkImageView vk_image_view_{};

    /// Cache the host visible buffer used to read/write the texture.
    /// (Mipmap level is not considered for the buffer size yet.)
    VkBuffer vk_staging_buffer_{};
    VkDeviceMemory vk_staging_buffer_memory_{};

    /// For releasing resources in destructor.
    /// This is null for wrapped external textures.
    VkDevice vk_device_{};

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

#endif // PATHFINDER_GPU_TEXTURE_VK_H
