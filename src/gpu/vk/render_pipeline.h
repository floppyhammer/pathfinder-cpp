#ifndef PATHFINDER_GPU_RENDER_PIPELINE_VK_H
#define PATHFINDER_GPU_RENDER_PIPELINE_VK_H

#include <memory>
#include <utility>

#include "../render_pipeline.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class RenderPipelineVk : public RenderPipeline {
    friend class DeviceVk;

public:
    ~RenderPipelineVk() override {
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
    RenderPipelineVk(VkDevice _vk_device,
                     const std::vector<VertexInputAttributeDescription> &_attribute_descriptions,
                     BlendState _blend_state,
                     std::string _label)
        : RenderPipeline(_attribute_descriptions, _blend_state, std::move(_label)), vk_device(_vk_device) {}

private:
    VkPipeline vk_pipeline{};

    VkDescriptorSetLayout vk_descriptor_set_layout{};

    VkPipelineLayout vk_layout{};

    VkDevice vk_device{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_RENDER_PIPELINE_VK_H
