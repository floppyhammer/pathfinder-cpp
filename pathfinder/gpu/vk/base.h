#ifndef PATHFINDER_GPU_DATA_VK_H
#define PATHFINDER_GPU_DATA_VK_H

#include <array>

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "../base.h"

// Vulkan header.
#ifdef __ANDROID__
    #include "vulkan_wrapper.h"
#else
    // Prevent the GLFW header from including the OpenGL header.
    #define GLFW_INCLUDE_NONE
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

#define VK_CHECK_RESULT(f)                                                                                       \
    {                                                                                                            \
        VkResult res = (f);                                                                                      \
        if (res != VK_SUCCESS) {                                                                                 \
            std::ostringstream string_stream;                                                                    \
            string_stream << "Fatal : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__; \
            Pathfinder::Logger::error(string_stream.str(), "Vulkan");                                            \
            assert(res == VK_SUCCESS);                                                                           \
        }                                                                                                        \
    }

namespace Pathfinder {

inline VkFormat to_vk_texture_format(TextureFormat texture_format) {
    switch (texture_format) {
        case TextureFormat::Rgba8Unorm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::Bgra8Unorm:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::Rgba8Srgb:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::Bgra8Srgb:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::Rgba16Float:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        default:
            abort();
    }
}

inline TextureFormat vk_to_texture_format(VkFormat texture_format) {
    switch (texture_format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return TextureFormat::Rgba8Unorm;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return TextureFormat::Bgra8Unorm;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return TextureFormat::Rgba8Srgb;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return TextureFormat::Bgra8Srgb;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return TextureFormat::Rgba16Float;
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
        case ShaderStage::VertexAndFragment:
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

inline VkFilter to_vk_sampler_filter(SamplerFilter filter) {
    switch (filter) {
        case SamplerFilter::Nearest:
            return VK_FILTER_NEAREST;
        case SamplerFilter::Linear:
            return VK_FILTER_LINEAR;
        default:
            abort();
    }
}

inline VkSamplerAddressMode to_vk_sampler_address_mode(SamplerAddressMode address_mode) {
    switch (address_mode) {
        case SamplerAddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerAddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
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
        case TextureLayout::Undefined:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case TextureLayout::PresentSrc:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case TextureLayout::ShaderReadOnly:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TextureLayout::TransferSrc:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case TextureLayout::TransferDst:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case TextureLayout::General:
            return VK_IMAGE_LAYOUT_GENERAL;
        case TextureLayout::ColorAttachment:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        default:
            abort();
    }
}

inline VkFormat to_vk_format(DataType type, uint32_t count) {
    switch (type) {
        case DataType::i8:
            return vk_formats[count - 1];
        case DataType::u8:
            return vk_formats[4 + count - 1];
        case DataType::i16:
            return vk_formats[8 + count - 1];
        case DataType::u16:
            return vk_formats[12 + count - 1];
        case DataType::i32:
            return vk_formats[16 + count - 1];
        case DataType::u32:
            return vk_formats[20 + count - 1];
        case DataType::f32:
            return vk_formats[24 + count - 1];
        case DataType::f16:
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
        default:
            abort();
    }
}

inline VkMemoryPropertyFlagBits to_vk_memory_property(MemoryProperty property) {
    switch (property) {
        case MemoryProperty::HostVisibleAndCoherent:
            return static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        case MemoryProperty::DeviceLocal:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        default:
            abort();
    }
}

inline VkAttachmentLoadOp to_vk_attachment_load_op(AttachmentLoadOp load_op) {
    switch (load_op) {
        case AttachmentLoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        default:
            abort();
    }
}

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DATA_VK_H
