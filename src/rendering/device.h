//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_DEVICE_H
#define PATHFINDER_DEVICE_H

#include "buffer.h"
#include "../common/math/basic.h"
#include "../common/global_macros.h"
#include "../common/logger.h"

#include <vector>

namespace Pathfinder {
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

    class Device {
    public:
        Device() = default;

        ~Device() = default;

        static std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size);

        /**
         * Upload to buffer.
         * @param buffer
         * @param offset
         * @param data_size Size of the data we are uploading, not the size of the buffer.
         * @param data
         */
        static void upload_to_buffer(const std::shared_ptr<Buffer>& buffer, size_t offset, size_t data_size, void *data);

        static void check_error(const char *flag);

        static void print_string(const char *name, GLenum s);

#ifdef PATHFINDER_USE_D3D11
        public:
            /// Allocate storage buffer.
            template<typename T>
            static uint64_t allocate_general_buffer(size_t size) {
                auto byte_size = size * sizeof(T);

                if (byte_size < MAX_BUFFER_SIZE_CLASS) {
                    byte_size = upper_power_of_two(byte_size);
                }

                GLuint buffer_id;
                glGenBuffers(1, &buffer_id);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
                glBufferData(GL_SHADER_STORAGE_BUFFER, byte_size, nullptr, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.

                check_error("allocate_general_buffer");

                return buffer_id;
            }

            /// Upload data to general buffer.
            template<typename T>
            static void upload_to_general_buffer(uint64_t buffer_id, size_t position, void *data, size_t size) {
                auto byte_size = size * sizeof(T);
                auto offset = position * sizeof(T);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byte_size, data);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.

                Device::check_error("upload_to_general_buffer");
            }

            /// Read data from general buffer.
            template<typename T>
            static void read_general_buffer(uint64_t buffer_id, size_t position, void *data, size_t size) {
                auto byte_size = size * sizeof(T);
                auto offset = position * sizeof(T);

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);

#ifdef __ANDROID__
                void *ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, byte_size, GL_MAP_READ_BIT);

                if (ptr) {
                    memcpy(data, ptr, byte_size);
                }

                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
#else
                glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, byte_size, data);
#endif

                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.

                Device::check_error("read_general_buffer");
            }

            /// Delete general buffer.
            static void free_general_buffer(GLuint buffer_id) {
                glDeleteBuffers(1, &buffer_id);
            }
#endif
    };
}

#endif //PATHFINDER_DEVICE_H
