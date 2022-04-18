#ifndef PATHFINDER_HAL_RENDER_PIPELINE_VK_H
#define PATHFINDER_HAL_RENDER_PIPELINE_VK_H

#include "../render_pipeline.h"

#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPipelineVk : public RenderPipeline {
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

    private:
        VkPipeline id;

        std::vector<VertexInputAttributeDescription> attribute_descriptions;

        ColorBlendState blend_state{};

        VkDevice device;

        friend class DeviceVk;
    };
}

#endif

#endif //PATHFINDER_HAL_RENDER_PIPELINE_VK_H
