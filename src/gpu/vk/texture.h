#ifndef PATHFINDER_GPU_TEXTURE_VK_H
#define PATHFINDER_GPU_TEXTURE_VK_H

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "../../common/math/rect.h"
#include "../texture.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class TextureVk : public Texture {
    friend class DriverVk;

public:
    // Actual construction is done by DriverVk.
    TextureVk(VkDevice p_device, uint32_t p_width, uint32_t p_height, TextureFormat p_format);

    // Actual deconstruction is done by itself.
    ~TextureVk();

    VkImage get_image() const;

    VkImageView get_image_view() const;

    VkSampler get_sampler() const;

    inline TextureLayout get_layout() const {
        return layout;
    }

    inline void set_layout(TextureLayout new_layout) {
        layout = new_layout;
    }

private:
    /// Handle.
    VkImage image{};

    /// Device memory.
    VkDeviceMemory image_memory{};

    /// Thin wrapper over image.
    VkImageView image_view{};

    /// How image should be filtered.
    VkSampler sampler{};

    /// For releasing resources in destructor.
    VkDevice device{};

    /// The initial layout must be undefined.
    TextureLayout layout = TextureLayout::Undefined;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_TEXTURE_VK_H
