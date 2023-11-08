#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H

#include <memory>
#include <utility>

#include "../compute_pipeline.h"

namespace Pathfinder {

class ComputePipelineVk : public ComputePipeline {
    friend class DeviceVk;

public:
    ~ComputePipelineVk() {
        vkDestroyDescriptorSetLayout(vk_device, vk_descriptor_set_layout, nullptr);
        vkDestroyPipeline(vk_device, vk_pipeline, nullptr);
        vkDestroyPipelineLayout(vk_device, vk_layout, nullptr);
    }

    inline VkPipeline get_pipeline() const {
        return vk_pipeline;
    }

    inline VkPipelineLayout get_layout() const {
        return vk_layout;
    }

    inline VkDescriptorSetLayout get_descriptor_set_layout() const {
        return vk_descriptor_set_layout;
    }

private:
    ComputePipelineVk(VkDevice _vk_device, std::string _label) : vk_device(_vk_device) {
        label = std::move(_label);
    }

private:
    VkPipeline vk_pipeline{};

    VkDescriptorSetLayout vk_descriptor_set_layout{};

    VkPipelineLayout vk_layout{};

    VkDevice vk_device{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H
