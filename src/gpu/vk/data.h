#ifndef PATHFINDER_GPU_DATA_VK_H
#define PATHFINDER_GPU_DATA_VK_H

#include "../data.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    VkFormat to_vk_texture_format(TextureFormat texture_format) {
        switch (texture_format) {
            case TextureFormat::RGBA:
                return VK_FORMAT_R8G8B8_SRGB;
            case TextureFormat::RGBA8:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case TextureFormat::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
    }

    VkShaderStageFlagBits to_vk_shader_stage(ShaderType shader_type) {
        switch (shader_type) {
            case ShaderType::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderType::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderType::Compute:
                return VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }

    VkDescriptorType to_vk_descriptor_type(DescriptorType descriptor_type) {
        switch (descriptor_type) {
            case DescriptorType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorType::Texture:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case DescriptorType::GeneralBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case DescriptorType::Image:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case DescriptorType::Max:
                abort();
        }
    }
}

#endif

#endif //PATHFINDER_GPU_DATA_VK_H
