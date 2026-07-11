#pragma once

#include "../descriptor_set.h"
#include "base.h"

namespace Pathfinder {

class DescriptorSetLayoutVk : public DescriptorSetLayout {
    friend class DeviceVk;

public:
    ~DescriptorSetLayoutVk() override {
        vkDestroyDescriptorSetLayout(vk_device_, vk_layout_, nullptr);
    }

    VkDescriptorSetLayout get_vk_layout() const {
        return vk_layout_;
    }

private:
    DescriptorSetLayoutVk(VkDevice device, VkDescriptorSetLayout layout, const std::vector<DescriptorLayout>& layouts)
        : DescriptorSetLayout(layouts) {
        vk_device_ = device;
        vk_layout_ = layout;
    }

private:
    VkDevice vk_device_{};
    VkDescriptorSetLayout vk_layout_{};
};

class DescriptorSetVk : public DescriptorSet {
    friend class DeviceVk;

public:
    ~DescriptorSetVk() override;

    void update_vk_descriptor_set(VkDevice vk_device, VkDescriptorSetLayout vk_descriptor_set_layout);

    VkDescriptorSet& get_vk_descriptor_set();

private:
    explicit DescriptorSetVk(const std::shared_ptr<DescriptorSetLayout>& layout) : DescriptorSet(layout) {}

    VkDescriptorPool vk_descriptor_pool_{};
    VkDescriptorSet vk_descriptor_set_{};

    bool descriptor_set_allocated_ = false;

    VkDevice vk_device_{};
};

} // namespace Pathfinder
