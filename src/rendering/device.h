//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_DEVICE_H
#define PATHFINDER_DEVICE_H

#include "../common/math/basic.h"
#include "../common/global_macros.h"
#include "../common/logger.h"
#include "buffer.h"

#include <vector>

namespace Pathfinder {
    // (For general buffer) Everything above 16 MB is allocated exactly.
    const uint64_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

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

    struct FramebufferDescriptor {
        uint32_t framebuffer_id{};
        int width{};
        int height{};
        GLenum blend_src{};
        GLenum blend_dst{};
        bool clear = false;

        FramebufferDescriptor() = default;
        FramebufferDescriptor(uint32_t p_framebuffer_id,
                              int p_width,
                              int p_height,
                              GLenum p_blend_src,
                              GLenum p_blend_dst,
                              bool p_clear) : framebuffer_id(p_framebuffer_id),
                                              width(p_width),
                                              height(p_height),
                                              blend_src(p_blend_src),
                                              blend_dst(p_blend_dst),
                                              clear(p_clear) {}
    };

    class Device {
    public:
        Device() = default;

        ~Device() = default;

        static void upload_to_vertex_buffer(uint32_t vbo, size_t size, void *data) {


            check_error("upload_to_vertex_buffer");
        }

        static std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) {
            auto buffer = std::make_shared<Buffer>();

            switch (type) {
                case BufferType::Uniform: {
                    unsigned int buffer_id;
                    glGenBuffers(1, &buffer_id);
                    glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
                    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

                    buffer->type = BufferType::Uniform;
                    buffer->size = size;
                    buffer->args.uniform.ubo = buffer_id;
                } break;
                case BufferType::Vertex: {
                    unsigned int buffer_id;
                    glGenBuffers(1, &buffer_id);
                    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
                    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

                    buffer->type = BufferType::Vertex;
                    buffer->size = size;
                    buffer->args.vertex.vbo = buffer_id;
                }
                    break;
                case BufferType::General: {
                    if (size < MAX_BUFFER_SIZE_CLASS) {
                        size = upper_power_of_two(size);
                    }

                    GLuint buffer_id;
                    glGenBuffers(1, &buffer_id);

                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer_id);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.

                    buffer->type = BufferType::General;
                    buffer->size = size;
                    buffer->args.general.sbo = buffer_id;
                }
                    break;
            }

            check_error("create_buffer");
            return buffer;
        }

        /**
         * Upload to buffer.
         * @param buffer
         * @param offset
         * @param data_size Size of the data we are uploading, not the size of the buffer.
         * @param data
         */
        static void upload_to_buffer(const std::shared_ptr<Buffer>& buffer, size_t offset, size_t data_size, void *data) {
            switch (buffer->type) {
                case BufferType::Uniform: {
                    glBindBuffer(GL_UNIFORM_BUFFER, buffer->args.uniform.ubo);
                    glBufferSubData(GL_UNIFORM_BUFFER, offset, data_size, data);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0); // Unbind.
                } break;
                case BufferType::Vertex: {
                    glBindBuffer(GL_ARRAY_BUFFER, buffer->args.vertex.vbo);
                    glBufferSubData(GL_ARRAY_BUFFER, offset, data_size, data);
                    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind.
                }
                    break;
                case BufferType::General: {
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->args.general.sbo);
                    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, data_size, data);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.
                }
                    break;
            }

            check_error("upload_to_buffer");
        }

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
