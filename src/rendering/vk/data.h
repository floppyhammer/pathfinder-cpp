#ifndef PATHFINDER_HAL_DATA_VK_H
#define PATHFINDER_HAL_DATA_VK_H

#include "../data.h"
#include "../../common/global_macros.h"

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

#endif //PATHFINDER_HAL_DATA_VK_H
