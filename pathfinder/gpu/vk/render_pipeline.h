#ifndef PATHFINDER_GPU_RENDER_PIPELINE_VK_H
#define PATHFINDER_GPU_RENDER_PIPELINE_VK_H

#include <memory>
#include <utility>

#include "../render_pipeline.h"

namespace Pathfinder {

class RenderPipelineVk : public RenderPipeline {
    friend class DeviceVk;

public:
    ~RenderPipelineVk() override {
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
    RenderPipelineVk(VkDevice vk_device,
                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                     BlendState blend_state,
                     std::string label)
        : RenderPipeline(attribute_descriptions, blend_state, std::move(label)), vk_device_(vk_device) {}

    VkPipeline vk_pipeline_{};

    VkDescriptorSetLayout vk_descriptor_set_layout_{};

    VkPipelineLayout vk_layout_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_RENDER_PIPELINE_VK_H
