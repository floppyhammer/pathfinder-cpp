#ifndef PATHFINDER_GPU_TEXTURE_VK_H
#define PATHFINDER_GPU_TEXTURE_VK_H

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "../../common/math/rect.h"
#include "../texture.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class TextureVk : public Texture {
    friend class DeviceVk;

public:
    // This constructor is only a wrapper, actual GPU resource allocation is done by DeviceVk.
    TextureVk(VkDevice _vk_device, const TextureDescriptor& _desc);

    // Releasing GPU resources is done by itself.
    ~TextureVk();

    // Wrapping an external Vulkan image.
    static std::shared_ptr<TextureVk> from_wrapping(const TextureDescriptor& _desc,
                                                    VkImage image,
                                                    VkDeviceMemory image_memory,
                                                    VkImageView image_view,
                                                    VkSampler sampler,
                                                    TextureLayout layout);

    VkImage get_image() const;

    VkImageView get_image_view() const;

    VkSampler get_sampler() const;

    TextureLayout get_layout() const;

    void set_layout(TextureLayout new_layout);

    void set_label(const std::string& _label) override;

private:
    /// Handle.
    VkImage vk_image{};

    /// Device memory.
    VkDeviceMemory vk_image_memory{};

    /// Thin wrapper over image.
    VkImageView vk_image_view{};

    /// How image should be filtered.
    VkSampler vk_sampler{};

    /// For releasing resources in destructor.
    VkDevice vk_device{};

    /// The initial layout must be undefined.
    TextureLayout layout = TextureLayout::Undefined;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_TEXTURE_VK_H
