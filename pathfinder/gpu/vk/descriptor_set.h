#ifndef PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
#define PATHFINDER_GPU_DESCRIPTOR_SET_VK_H

#include "../descriptor_set.h"
#include "base.h"

namespace Pathfinder {

class DescriptorSetVk : public DescriptorSet {
    friend class DeviceVk;

public:
    ~DescriptorSetVk() override;

    void update_vk_descriptor_set(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout);

    VkDescriptorSet &get_vk_descriptor_set();

private:
    DescriptorSetVk() = default;

    VkDescriptorPool vk_descriptor_pool_{};
    VkDescriptorSet vk_descriptor_set_{};

    bool descriptor_set_allocated_ = false;

    VkDevice vk_device_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DESCRIPTOR_SET_VK_H
