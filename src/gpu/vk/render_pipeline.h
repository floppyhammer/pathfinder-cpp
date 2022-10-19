#ifndef PATHFINDER_GPU_RENDER_PIPELINE_VK_H
#define PATHFINDER_GPU_RENDER_PIPELINE_VK_H

#include <memory>
#include <utility>

#include "../render_pipeline.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class RenderPipelineVk : public RenderPipeline {
    friend class DriverVk;

public:
    RenderPipelineVk(VkDevice p_device,
                     std::vector<VertexInputAttributeDescription> p_attribute_descriptions,
                     ColorBlendState p_blend_state)
        : RenderPipeline(std::move(p_attribute_descriptions), p_blend_state), device(p_device) {}

    ~RenderPipelineVk() {
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

#endif // PATHFINDER_GPU_RENDER_PIPELINE_VK_H
