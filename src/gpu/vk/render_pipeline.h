#ifndef PATHFINDER_GPU_RENDER_PIPELINE_VK_H
#define PATHFINDER_GPU_RENDER_PIPELINE_VK_H

#include "../render_pipeline.h"

#include <memory>
#include <utility>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class RenderPipelineVk : public RenderPipeline {
        friend class DriverVk;

    public:
        RenderPipelineVk(VkDevice p_device,
                         std::vector<VertexInputAttributeDescription> p_attribute_descriptions,
                         ColorBlendState p_blend_state)
                : device(p_device),
                  attribute_descriptions(std::move(p_attribute_descriptions)),
                  blend_state(p_blend_state) {}

        ~RenderPipelineVk() {
            vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
            vkDestroyPipeline(device, vk_pipeline, nullptr);
            vkDestroyPipelineLayout(device, layout, nullptr);
        }

        inline const std::vector<VertexInputAttributeDescription> &get_attribute_descriptions() const {
            return attribute_descriptions;
        }

        inline ColorBlendState get_blend_state() const {
            return blend_state;
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

        std::vector<VertexInputAttributeDescription> attribute_descriptions;

        ColorBlendState blend_state{};

        VkDevice device{};
    };
}

#endif

#endif //PATHFINDER_GPU_RENDER_PIPELINE_VK_H
