#ifndef PATHFINDER_GPU_DATA_GL_H
#define PATHFINDER_GPU_DATA_GL_H

#include "../../common/global_macros.h"
#include "../data.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
inline GLint to_gl_blend_factor(BlendFactor blend_factor) {
    switch (blend_factor) {
        case BlendFactor::ONE:
            return GL_ONE;
        case BlendFactor::ONE_MINUS_SRC_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
    }
}

inline GLint to_gl_data_type(DataType data_type) {
    switch (data_type) {
        case DataType::BYTE:
            return GL_BYTE;
        case DataType::UNSIGNED_BYTE:
            return GL_UNSIGNED_BYTE;
        case DataType::SHORT:
            return GL_SHORT;
        case DataType::UNSIGNED_SHORT:
            return GL_UNSIGNED_SHORT;
        case DataType::INT:
            return GL_INT;
        case DataType::UNSIGNED_INT:
            return GL_UNSIGNED_INT;
        case DataType::FLOAT:
            return GL_FLOAT;
        case DataType::HALF_FLOAT:
            return GL_HALF_FLOAT;
    }
}

inline GLint to_gl_texture_format(TextureFormat texture_format) {
    switch (texture_format) {
        case TextureFormat::RGBA8_UNORM:
            return GL_RGBA8;
        case TextureFormat::RGBA8_SRGB:
            return GL_SRGB8;
        case TextureFormat::RGBA16F:
            return GL_RGBA16F;
        default:
            return GL_RGBA8;
    }
}
} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_DATA_GL_H
