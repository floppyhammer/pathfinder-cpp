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
            default:
                abort();
        }
    }

    VkShaderStageFlagBits to_vk_shader_stage(ShaderType shader_type) {
        switch (shader_type) {
            case ShaderType::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderType::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderType::VertexFragment:
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderType::Compute:
                return VK_SHADER_STAGE_COMPUTE_BIT;
            default:
                abort();
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
            default:
                abort();
        }
    }

//    // Integers.
//    BYTE, // 1 byte
//    UNSIGNED_BYTE, // 1 byte
//    SHORT, // 2 bytes
//    UNSIGNED_SHORT, // 2 bytes
//    INT, // 4 bytes
//    UNSIGNED_INT, // 4 bytes
//
//    // Floats.
//    FLOAT, // 4 bytes
//    HALF_FLOAT, // 2 bytes
//
    std::array<VkFormat, 32> vk_formats = {
            VK_FORMAT_R8_SINT, VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8A8_SINT,
            VK_FORMAT_R8_UINT, VK_FORMAT_R8G8_UINT, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8A8_UINT,
            VK_FORMAT_R16_SINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16A16_SINT,
            VK_FORMAT_R16_UINT, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16A16_UINT,
            VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT,
            VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT,
            VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_R16_SFLOAT, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT,
    };

    VkFormat to_vk_format(DataType type, uint32_t count) {
        switch (type) {
            case DataType::BYTE:
                return vk_formats[count - 1];
            case DataType::UNSIGNED_BYTE:
                return vk_formats[4 + count - 1];
            case DataType::SHORT:
                return vk_formats[8 + count - 1];
            case DataType::UNSIGNED_SHORT:
                return vk_formats[12 + count - 1];
            case DataType::INT:
                return vk_formats[16 + count - 1];
            case DataType::UNSIGNED_INT:
                return vk_formats[20 + count - 1];
            case DataType::FLOAT:
                return vk_formats[24 + count - 1];
            case DataType::HALF_FLOAT:
                return vk_formats[28 + count - 1];
            default:
                abort();
        }
    }
}

#endif

#endif //PATHFINDER_GPU_DATA_VK_H
