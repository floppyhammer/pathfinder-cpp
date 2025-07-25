#ifndef PATHFINDER_GPU_DATA_GL_H
#define PATHFINDER_GPU_DATA_GL_H

#include "../../common/global_macros.h"
#include "../base.h"
#include "../render_api.h"

namespace Pathfinder {

inline GLint to_gl_blend_factor(BlendFactor blend_factor) {
    switch (blend_factor) {
        case BlendFactor::One:
            return GL_ONE;
        case BlendFactor::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        default:
            abort();
    }
}

inline GLint to_gl_data_type(DataType data_type) {
    switch (data_type) {
        case DataType::i8:
            return GL_BYTE;
        case DataType::u8:
            return GL_UNSIGNED_BYTE;
        case DataType::i16:
            return GL_SHORT;
        case DataType::u16:
            return GL_UNSIGNED_SHORT;
        case DataType::i32:
            return GL_INT;
        case DataType::u32:
            return GL_UNSIGNED_INT;
        case DataType::f16:
            return GL_HALF_FLOAT;
        case DataType::f32:
            return GL_FLOAT;
        default:
            abort();
    }
}

inline GLint to_gl_texture_format(TextureFormat texture_format) {
    switch (texture_format) {
        case TextureFormat::R8:
            return GL_R8;
        case TextureFormat::Rg8:
            return GL_RG8;
        case TextureFormat::Rgba8Unorm:
            return GL_RGBA8;
        case TextureFormat::Rgba8Srgb:
            return GL_SRGB8;
        case TextureFormat::Rgba16Float:
            return GL_RGBA16F;
        default:
            abort();
    }
}

inline GLint to_gl_pixel_data_format(TextureFormat texture_format) {
    switch (texture_format) {
        case TextureFormat::R8:
            return GL_RED;
        case TextureFormat::Rg8:
            return GL_RG;
        case TextureFormat::Rgba8Unorm:
        case TextureFormat::Rgba8Srgb:
        case TextureFormat::Rgba16Float:
            return GL_RGBA;
        default:
            abort();
    }
}

inline GLint to_gl_sampler_filter(SamplerFilter filter) {
    switch (filter) {
        case SamplerFilter::Nearest:
            return GL_NEAREST;
        case SamplerFilter::Linear:
            return GL_LINEAR;
        default:
            abort();
    }
}

inline GLint to_gl_sampler_address_mode(SamplerAddressMode address_mode) {
    switch (address_mode) {
        case SamplerAddressMode::Repeat:
            return GL_REPEAT;
        case SamplerAddressMode::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case SamplerAddressMode::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
            // Not available in GLES until 3.2.
            //        case SamplerAddressMode::ClampToBorder:
            //            return GL_CLAMP_TO_BORDER;
        default:
            abort();
    }
}

} // namespace Pathfinder

#endif // PATHFINDER_GPU_DATA_GL_H
