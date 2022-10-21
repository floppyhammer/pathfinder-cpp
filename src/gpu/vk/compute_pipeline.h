#ifndef PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H
#define PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H

#include <memory>
#include <utility>

#include "../compute_pipeline.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
class ComputePipelineVk : public ComputePipeline {
    friend class DriverVk;

public:
    ComputePipelineVk(VkDevice p_device) : device(p_device) {}

    ~ComputePipelineVk() {
        vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
        vkDestroyPipeline(device, vk_pipeline, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
    }

    inline VkPipeline get_pipeline() const {
        return vk_pipeline;
    }

    inline VkPipelineLayout get_layout() const {
        return layout;
    }

    inline VkDescriptorSetLayout get_descriptor_set_layout() const {
        return descriptor_set_layout;
    }

private:
    VkPipeline vk_pipeline{};

    VkDescriptorSetLayout descriptor_set_layout{};

    VkPipelineLayout layout{};

    VkDevice device{};
};
} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMPUTE_PIPELINE_VK_H
