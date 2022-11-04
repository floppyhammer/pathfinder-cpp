#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_VK_H

#include <cstdint>
#include <unordered_map>

#include "../../common/global_macros.h"
#include "../descriptor_set.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class DescriptorSetVk : public DescriptorSet {
    friend class DriverVk;

public:
    DescriptorSetVk() = default;

    ~DescriptorSetVk();

    void update_vk_descriptor_set(VkDevice _device, VkDescriptorSetLayout descriptor_set_layout);

    VkDescriptorSet &get_vk_descriptor_set();

private:
    VkDescriptorPool descriptor_pool{};
    VkDescriptorSet descriptor_set{};

    bool descriptor_set_allocated = false;

    VkDevice device{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
