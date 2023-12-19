#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H

#include <memory>
#include <utility>

#include "../compute_pipeline.h"

namespace Pathfinder {

class ComputePipelineVk : public ComputePipeline {
    friend class DeviceVk;

public:
    ~ComputePipelineVk() override {
        vkDestroyDescriptorSetLayout(vk_device_, vk_descriptor_set_layout_, nullptr);
        vkDestroyPipeline(vk_device_, vk_pipeline_, nullptr);
        vkDestroyPipelineLayout(vk_device_, vk_layout_, nullptr);
    }

    VkPipeline get_pipeline() const {
        return vk_pipeline_;
    }

    VkPipelineLayout get_layout() const {
        return vk_layout_;
    }

    VkDescriptorSetLayout get_descriptor_set_layout() const {
        return vk_descriptor_set_layout_;
    }

private:
    ComputePipelineVk(VkDevice vk_device, std::string label) : vk_device_(vk_device) {
        label_ = std::move(label);
    }

    VkPipeline vk_pipeline_{};

    VkDescriptorSetLayout vk_descriptor_set_layout_{};

    VkPipelineLayout vk_layout_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H
