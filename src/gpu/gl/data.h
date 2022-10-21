#ifndef PATHFINDER_GPU_DATA_GL_H
#define PATHFINDER_GPU_DATA_GL_H

#include "../../common/global_macros.h"
#include "../data.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

inline GLint to_gl_blend_factor(BlendFactor blend_factor) {
    switch (blend_factor) {
        case BlendFactor::One:
            return GL_ONE;
        case BlendFactor::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
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
    }
}

inline GLint to_gl_texture_format(TextureFormat texture_format) {
    switch (texture_format) {
        case TextureFormat::Rgba8Unorm:
            return GL_RGBA8;
        case TextureFormat::Rgba8Srgb:
            return GL_SRGB8;
        case TextureFormat::Rgba16Float:
            return GL_RGBA16F;
        default:
            return GL_RGBA8;
    }
}

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_DATA_GL_H
