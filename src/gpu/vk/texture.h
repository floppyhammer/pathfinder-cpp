#ifndef PATHFINDER_GPU_TEXTURE_VK_H
#define PATHFINDER_GPU_TEXTURE_VK_H

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "../../common/math/rect.h"
#include "../texture.h"
#include "base.h"

namespace Pathfinder {

class TextureVk : public Texture {
    friend class DeviceVk;

public:
    // Releasing GPU resources is done by itself.
    ~TextureVk() override;

    // Wrapping an external Vulkan image.
    static std::shared_ptr<TextureVk> from_wrapping(const TextureDescriptor& _desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    TextureLayout layout);

    VkImage get_image() const;

    VkImageView get_image_view() const;

    TextureLayout get_layout() const;

    void set_layout(TextureLayout new_layout);

    void set_label(const std::string& _label) override;

private:
    // This constructor is only a wrapper, actual GPU resource allocation is done by DeviceVk.
    TextureVk(VkDevice _vk_device, const TextureDescriptor& _desc);

private:
    /// Handle.
    VkImage vk_image{};

    /// Device memory.
    VkDeviceMemory vk_image_memory{};

    /// Thin wrapper over image.
    VkImageView vk_image_view{};

    /// For releasing resources in destructor.
    /// This is null for wrapped external textures.
    VkDevice vk_device{};

    /// The initial layout must be undefined.
    TextureLayout layout = TextureLayout::Undefined;
};

class SamplerVk : public Sampler {
    friend class DeviceVk;

public:
    ~SamplerVk() override {
        vkDestroySampler(vk_device, vk_sampler, nullptr);
    }

    VkSampler get_sampler() const {
        return vk_sampler;
    }

private:
    SamplerVk(SamplerDescriptor descriptor, VkSampler _vk_sampler, VkDevice _vk_device) : Sampler(descriptor) {
        vk_sampler = _vk_sampler;
        vk_device = _vk_device;
    }

private:
    VkSampler vk_sampler{};

    VkDevice vk_device{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_TEXTURE_VK_H
