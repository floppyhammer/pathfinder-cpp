#ifndef PATHFINDER_GPU_RENDER_PIPELINE_VK_H
#define PATHFINDER_GPU_RENDER_PIPELINE_VK_H

#include "../render_pipeline.h"

#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPipelineVk : public RenderPipeline {
        friend class DriverVk;
    public:
        RenderPipelineVk(VkDevice p_device,
                         const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
                         ColorBlendState p_blend_state) {
            device = p_device;

            attribute_descriptions = p_attribute_descriptions;

            blend_state = p_blend_state;
        }

        ~RenderPipelineVk() {
            vkDestroyPipeline(device, id, nullptr);
        }

        inline const std::vector<VertexInputAttributeDescription> &get_attribute_descriptions() const {
            return attribute_descriptions;
        }

        inline ColorBlendState get_blend_state() const {
            return blend_state;
        }

        inline VkPipeline get_pipeline() const {
            return id;
        }

        inline VkPipelineLayout get_layout() const {
            return layout;
        }

    private:
        VkPipeline id;

        VkDescriptorSetLayout descriptor_set_layout;

        VkPipelineLayout layout;

        std::vector<VertexInputAttributeDescription> attribute_descriptions;

        ColorBlendState blend_state{};

        VkDevice device;
    };
}

#endif

#endif //PATHFINDER_GPU_RENDER_PIPELINE_VK_H
