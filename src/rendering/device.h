//
// Created by floppyhammer on 6/15/2021.
//

#ifndef PATHFINDER_DEVICE_H
#define PATHFINDER_DEVICE_H

#include "../common/global_macros.h"

#include <cstdint>
#include <memory>

namespace Pathfinder {
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

    class Device {
    public:
        Device() = default;

        virtual ~Device() = 0;

        virtual uint64_t allocate_general_buffer(size_t p_size) = 0;

        virtual void upload_to_general_buffer(uint64_t buffer_id, size_t position, void *data, size_t data_size) = 0;

        virtual void read_general_buffer(uint64_t buffer_id, size_t position, void *data, size_t data_size) = 0;

        virtual void free_general_buffer(GLuint buffer_id) = 0;
    };
}

#endif //PATHFINDER_DEVICE_H
