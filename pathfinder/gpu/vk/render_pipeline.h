#pragma once

#include <memory>
#include <utility>

#include "../render_pipeline.h"

namespace Pathfinder {

class RenderPipelineVk : public RenderPipeline {
    friend class DeviceVk;

public:
    ~RenderPipelineVk() override {
        vkDestroyPipeline(vk_device_, vk_pipeline_, nullptr);
        vkDestroyPipelineLayout(vk_device_, vk_layout_, nullptr);
    }

    VkPipeline get_pipeline() const {
        return vk_pipeline_;
    }

    VkPipelineLayout get_layout() const {
        return vk_layout_;
    }

private:
    RenderPipelineVk(VkDevice vk_device,
                     const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                     BlendState blend_state,
                     std::string label)
        : RenderPipeline(attribute_descriptions, blend_state, std::move(label)), vk_device_(vk_device) {}
    // A walkaround of render pass dependency.
    std::shared_ptr<RenderPassVk> render_pass_vk_;

    VkPipeline vk_pipeline_{};

    VkPipelineLayout vk_layout_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder
