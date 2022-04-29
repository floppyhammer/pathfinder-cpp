#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_VK_H

#include "../descriptor_set.h"
#include "../../common/global_macros.h"

#include <cstdint>
#include <unordered_map>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DescriptorSetVk : public DescriptorSet {
        friend class DriverVk;
    public:
        ~DescriptorSetVk();

        void update_vk_descriptor_set(VkDevice p_device, VkDescriptorSetLayout descriptor_set_layout);

        VkDescriptorSet &get_vk_descriptor_set();

    private:
        DescriptorSetVk() = default;

        VkDescriptorPool descriptor_pool{};
        VkDescriptorSet descriptor_set{};

        bool descriptor_set_allocated = false;

        VkDevice device{};
    };
}

#endif

#endif //PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
