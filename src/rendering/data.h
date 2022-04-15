//
// Created by floppyhammer on 4/14/2022.
//

#ifndef PATHFINDER_DATA_H
#define PATHFINDER_DATA_H

#include "../common/global_macros.h"

namespace Pathfinder {
    // Everything above 16 MB is allocated exactly for general buffer.
    const uint64_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

    enum class DataType {
        // Integers.
        BYTE = GL_BYTE, // 1 byte
        UNSIGNED_BYTE = GL_UNSIGNED_BYTE, // 1 byte
        SHORT = GL_SHORT, // 2 bytes
        UNSIGNED_SHORT = GL_UNSIGNED_SHORT, // 2 bytes
        INT = GL_INT, // 4 bytes
        UNSIGNED_INT = GL_UNSIGNED_INT, // 4 bytes

        // Floats.
        FLOAT = GL_FLOAT, // 4 bytes
        HALF_FLOAT = GL_HALF_FLOAT, // 2 bytes
    };

    /// Texture format in GPU memory.
    enum class TextureFormat {
        RGBA = GL_RGBA,
        RGBA8 = GL_RGBA8,
        RGBA16F = GL_RGBA16F,
    };

    /// Texture format in CPU memory.
    enum class PixelDataFormat {
        RED = GL_RED,
        RGBA = GL_RGBA,
    };

    enum class ShaderType {
        Vertex,
        Fragment,
        Compute,
    };

    enum class DeviceType {
        GL3, // Or ES 3.0
        GL4, // Or ES 3.1
        Vulkan,
    };

    enum class BlendFactor {
        ONE = 0,
        ONE_MINUS_SRC_ALPHA = 1,
    };

    struct ColorBlendState {
        bool blend_enable;
        BlendFactor src_blend_factor;
        BlendFactor dst_blend_factor;
    };

    inline GLenum to_gl_blend_factor(BlendFactor blend_factor) {
        switch (blend_factor) {
            case BlendFactor::ONE:
                return GL_ONE;
            case BlendFactor::ONE_MINUS_SRC_ALPHA:
                return GL_ONE_MINUS_SRC_ALPHA;
        }
    }
}

#endif //PATHFINDER_DATA_H
