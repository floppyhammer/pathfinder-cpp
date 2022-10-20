#ifndef PATHFINDER_GPU_DATA_VK_H
#define PATHFINDER_GPU_DATA_VK_H

#include <array>

#include "../../common/global_macros.h"
#include "../data.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
inline VkFormat to_vk_texture_format(TextureFormat texture_format) {
    switch (texture_format) {
        case TextureFormat::RGBA8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::RGBA8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::BGRA8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::RGBA16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        default:
            abort();
    }
}

inline TextureFormat vk_to_texture_format(VkFormat texture_format) {
    switch (texture_format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return TextureFormat::RGBA8_UNORM;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return TextureFormat::BGRA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return TextureFormat::RGBA8_SRGB;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return TextureFormat::BGRA8_SRGB;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return TextureFormat::RGBA16F;
        default:
            abort();
    }
}

inline VkShaderStageFlagBits to_vk_shader_stage(ShaderStage shader_type) {
    switch (shader_type) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::VertexFragment:
            return static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            abort();
    }
}

inline VkDescriptorType to_vk_descriptor_type(DescriptorType descriptor_type) {
    switch (descriptor_type) {
        case DescriptorType::UniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::Sampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::StorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::Image:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        default:
            abort();
    }
}

const std::array<VkFormat, 32> vk_formats = {
    VK_FORMAT_R8_SINT,    VK_FORMAT_R8G8_SINT,     VK_FORMAT_R8G8B8_SINT,      VK_FORMAT_R8G8B8A8_SINT,
    VK_FORMAT_R8_UINT,    VK_FORMAT_R8G8_UINT,     VK_FORMAT_R8G8B8_UINT,      VK_FORMAT_R8G8B8A8_UINT,
    VK_FORMAT_R16_SINT,   VK_FORMAT_R16G16_SINT,   VK_FORMAT_R16G16B16_SINT,   VK_FORMAT_R16G16B16A16_SINT,
    VK_FORMAT_R16_UINT,   VK_FORMAT_R16G16_UINT,   VK_FORMAT_R16G16B16_UINT,   VK_FORMAT_R16G16B16A16_UINT,
    VK_FORMAT_R32_SINT,   VK_FORMAT_R32G32_SINT,   VK_FORMAT_R32G32B32_SINT,   VK_FORMAT_R32G32B32A32_SINT,
    VK_FORMAT_R32_UINT,   VK_FORMAT_R32G32_UINT,   VK_FORMAT_R32G32B32_UINT,   VK_FORMAT_R32G32B32A32_UINT,
    VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R16_SFLOAT, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT,
};

inline VkImageLayout to_vk_layout(TextureLayout layout) {
    switch (layout) {
        case TextureLayout::PRESENT_SRC:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case TextureLayout::SHADER_READ_ONLY:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TextureLayout::TRANSFER_SRC:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case TextureLayout::TRANSFER_DST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case TextureLayout::GENERAL:
            return VK_IMAGE_LAYOUT_GENERAL;
        case TextureLayout::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case TextureLayout::COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
}

inline VkFormat to_vk_format(DataType type, uint32_t count) {
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

inline VkBlendFactor to_vk_blend_factor(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }
}

inline VkMemoryPropertyFlagBits to_vk_memory_property(MemoryProperty property) {
    switch (property) {
        case MemoryProperty::HOST_VISIBLE_AND_COHERENT:
            return static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        case MemoryProperty::DEVICE_LOCAL:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
}

inline VkAttachmentLoadOp to_vk_attachment_load_op(AttachmentLoadOp load_op) {
    switch (load_op) {
        case AttachmentLoadOp::CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoadOp::LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
    }
}
} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_DATA_VK_H
