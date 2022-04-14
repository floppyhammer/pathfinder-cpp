//
// Created by floppyhammer on 4/14/2022.
//

#ifndef PATHFINDER_DATA_H
#define PATHFINDER_DATA_H

#include "../common/global_macros.h"

// (For general buffer) Everything above 16 MB is allocated exactly.
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
    FLOAT = GL_FLOAT,
    HALF_FLOAT = GL_HALF_FLOAT, // 2 bytes
};

/// Format in GPU memory.
enum class TextureFormat {
    RGBA = GL_RGBA,
    RGBA8 = GL_RGBA8,
    RGBA16F = GL_RGBA16F,
};

/// Format in CPU memory.
enum class PixelDataFormat {
    RED = GL_RED,
    RGBA = GL_RGBA,
};

enum class ShaderKind {
    Vertex,
    Fragment,
    Compute,
};

enum class DeviceType {
    GL3, // Or ES 3.0
    GL4, // Or ES 3.1
    Metal,
};

#endif //PATHFINDER_DATA_H
