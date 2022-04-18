#ifndef PATHFINDER_GPU_DATA_VK_H
#define PATHFINDER_GPU_DATA_VK_H

#include "../data.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    VkFormat to_vk_texture_format(TextureFormat texture_format) {
        switch (texture_format) {
            case TextureFormat::RGBA:
                return VK_FORMAT_R8G8B8_SRGB;
            case TextureFormat::RGBA8:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case TextureFormat::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
    }
}

#endif

#endif //PATHFINDER_GPU_DATA_VK_H
