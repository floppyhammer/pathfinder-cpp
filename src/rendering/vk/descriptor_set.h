#ifndef PATHFINDER_HAL_DESCRIPTOR_SET_VK_H
#define PATHFINDER_HAL_DESCRIPTOR_SET_VK_H

#include "../descriptor_set.h"
#include "../../common/global_macros.h"

#include <cstdint>
#include <unordered_map>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DescriptorSetVk : public DescriptorSet {
    public:
        void update_vk_descriptor_set();

    private:
        VkDescriptorSet descriptor_set;
    };
}

#endif

#endif //PATHFINDER_HAL_DESCRIPTOR_SET_VK_H
