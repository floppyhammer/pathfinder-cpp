#pragma once

#include <MetalKit/MetalKit.h>

#include <cassert>

#include "../base.h"

namespace Pathfinder {

inline MTLPixelFormat to_mtl_pixel_format(const TextureFormat tf) {
    switch (tf) {
        case TextureFormat::R8: {
            return MTLPixelFormatR8Unorm;
        }
        case TextureFormat::Rg8: {
            return MTLPixelFormatRG8Unorm;
        }
        case TextureFormat::Rgba8Unorm: {
            return MTLPixelFormatRGBA8Unorm;
        }
        case TextureFormat::Bgra8Unorm: {
            return MTLPixelFormatBGRA8Unorm;
        }
        case TextureFormat::Rgba8Srgb: {
            return MTLPixelFormatRGBA8Unorm_sRGB;
        }
        case TextureFormat::Bgra8Srgb: {
            return MTLPixelFormatBGRA8Unorm_sRGB;
        }
        case TextureFormat::Rgba16Float: {
            return MTLPixelFormatRGBA16Float;
        }
    }
}

inline TextureFormat from_mtl_pixel_format(const MTLPixelFormat mtl_pf) {
    TextureFormat fm = TextureFormat::R8;
    switch (mtl_pf) {
        case MTLPixelFormatR8Unorm:
            fm = TextureFormat::R8;
            break;
        case MTLPixelFormatRG8Unorm:
            fm = TextureFormat::Rg8;
            break;
        case MTLPixelFormatRGBA8Unorm:
            fm = TextureFormat::Rgba8Unorm;
            break;
        case MTLPixelFormatRGBA8Unorm_sRGB:
            fm = TextureFormat::Rgba8Srgb;
            break;
        case MTLPixelFormatBGRA8Unorm:
            fm = TextureFormat::Bgra8Unorm;
            break;
        case MTLPixelFormatBGRA8Unorm_sRGB:
            fm = TextureFormat::Bgra8Srgb;
            break;
        case MTLPixelFormatRGBA16Float:
            fm = TextureFormat::Rgba16Float;
            break;
        default:
            abort();
    }
    return fm;
}

inline MTLBlendFactor to_mtl_blend_factor(const BlendFactor factor) {
    switch (factor) {
        case BlendFactor::One:
            return MTLBlendFactorOne;
        case BlendFactor::OneMinusSrcAlpha:
            return MTLBlendFactorOneMinusSourceAlpha;
        default:
            abort();
    }
}

inline MTLBlendOperation to_mtl_blend_op(const BlendOperation op) {
    switch (op) {
        case BlendOperation::Add:
            return MTLBlendOperationAdd;
        default:
            abort();
    }
}

static const MTLVertexFormat MTL_FORMATS[] = {
    MTLVertexFormatChar,   MTLVertexFormatChar2,   MTLVertexFormatChar3,   MTLVertexFormatChar4,   //
    MTLVertexFormatUChar,  MTLVertexFormatUChar2,  MTLVertexFormatUChar3,  MTLVertexFormatUChar4,  //
    MTLVertexFormatShort,  MTLVertexFormatShort2,  MTLVertexFormatShort3,  MTLVertexFormatShort4,  //
    MTLVertexFormatUShort, MTLVertexFormatUShort2, MTLVertexFormatUShort3, MTLVertexFormatUShort4, //
    MTLVertexFormatInt,    MTLVertexFormatInt2,    MTLVertexFormatInt3,    MTLVertexFormatInt4,    //
    MTLVertexFormatUInt,   MTLVertexFormatUInt2,   MTLVertexFormatUInt3,   MTLVertexFormatUInt4,   //
    MTLVertexFormatFloat,  MTLVertexFormatFloat2,  MTLVertexFormatFloat3,  MTLVertexFormatFloat4,  //
    MTLVertexFormatHalf,   MTLVertexFormatHalf2,   MTLVertexFormatHalf3,   MTLVertexFormatHalf4,   //
};

inline MTLVertexFormat to_mtl_vertex_format(DataType type, uint32_t count) {
    switch (type) {
        case DataType::i8:
            return MTL_FORMATS[count - 1];
        case DataType::u8:
            return MTL_FORMATS[4 + count - 1];
        case DataType::i16:
            return MTL_FORMATS[8 + count - 1];
        case DataType::u16:
            return MTL_FORMATS[12 + count - 1];
        case DataType::i32:
            return MTL_FORMATS[16 + count - 1];
        case DataType::u32:
            return MTL_FORMATS[20 + count - 1];
        case DataType::f32:
            return MTL_FORMATS[24 + count - 1];
        case DataType::f16:
            return MTL_FORMATS[28 + count - 1];
        default:
            return MTLVertexFormatInvalid;
    }
}

inline MTLResourceOptions to_mtl_resource_option(const MemoryProperty memory_property) {
    switch (memory_property) {
        case MemoryProperty::HostVisibleAndCoherent:
            return MTLResourceStorageModeShared;
        case MemoryProperty::DeviceLocal:
            return MTLResourceStorageModePrivate;
        default:
            abort();
    }
}

/// For textures.
inline MTLStorageMode to_mtl_storage_mode(const MemoryProperty memory_property) {
    switch (memory_property) {
        case MemoryProperty::HostVisibleAndCoherent:
            return MTLStorageModeShared;
        case MemoryProperty::DeviceLocal:
            return MTLStorageModePrivate;
        default:
            abort();
    }
}

inline MTLSamplerMinMagFilter to_mtl_sampler_filter(const SamplerFilter filter) {
    return filter == SamplerFilter::Nearest ? MTLSamplerMinMagFilterNearest : MTLSamplerMinMagFilterLinear;
}

inline MTLSamplerAddressMode to_mtl_sampler_address_mode(const SamplerAddressMode address_mode) {
    switch (address_mode) {
        case SamplerAddressMode::Repeat:
            return MTLSamplerAddressModeRepeat;
        case SamplerAddressMode::MirroredRepeat:
            return MTLSamplerAddressModeMirrorRepeat;
        case SamplerAddressMode::ClampToEdge:
            return MTLSamplerAddressModeClampToEdge;
        case SamplerAddressMode::ClampToBorder:
            return MTLSamplerAddressModeClampToBorderColor;
        default:
            abort();
    }
}

inline MTLRenderStages to_mtl_render_stage(const ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:
            return MTLRenderStageVertex;
        case ShaderStage::Fragment:
            return MTLRenderStageFragment;
        default:
            assert(false);
            abort();
    }
}

} // namespace Pathfinder
