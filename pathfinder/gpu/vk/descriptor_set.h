#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_VK_H

#include <cstdint>
#include <unordered_map>

#include "../../common/global_macros.h"
#include "../descriptor_set.h"
#include "base.h"

namespace Pathfinder {

class DescriptorSetVk : public DescriptorSet {
    friend class DeviceVk;

public:
    ~DescriptorSetVk() override;

    void update_vk_descriptor_set(VkDevice _device, VkDescriptorSetLayout descriptor_set_layout);

    VkDescriptorSet &get_vk_descriptor_set();

private:
    DescriptorSetVk() = default;

private:
    VkDescriptorPool descriptor_pool{};
    VkDescriptorSet descriptor_set{};

    bool descriptor_set_allocated = false;

    VkDevice device{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
