//
// Created by floppyhammer on 8/26/2021.
//

#ifndef PATHFINDER_DEVICE_H
#define PATHFINDER_DEVICE_H

#include "../common/math/basic.h"
#include "../common/global_macros.h"
#include "../common/logger.h"

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

    enum class VertexStep {
        PER_VERTEX,
        PER_INSTANCE,
    };

    struct AttributeDescriptor {
        uint32_t vao;
        uint32_t vbo;
        uint8_t size; // Must be 1, 2, 3, 4.
        DataType type;
        uint32_t stride;
        size_t offset;
        VertexStep vertex_step;
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

        static void bind_framebuffer(const FramebufferDescriptor &descriptor) {
            glBindFramebuffer(GL_FRAMEBUFFER, descriptor.framebuffer_id);

            if (descriptor.clear) {
                glClearColor(0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);
            }

            glViewport(0, 0, descriptor.width, descriptor.height);

            glEnable(GL_BLEND);
            glBlendFunc(descriptor.blend_src, descriptor.blend_dst);

            check_error("bind_framebuffer");
        }

        static void upload_to_vertex_buffer(uint32_t vbo, size_t size, void *data) {
            // Bind the VAO first, then bind the VBO.
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);

            glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            check_error("upload_to_vertex_buffer");
        }

        static void create_uniform_buffer(uint32_t &ubo, size_t size) {
            glGenBuffers(1, &ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

            check_error("create_uniform_buffer");
        }

        static void upload_to_uniform_buffer(uint32_t ubo, size_t offset, size_t size, void *data) {
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);

            check_error("upload_to_uniform_buffer");
        }

        static void bind_attributes(std::vector<AttributeDescriptor> &attribute_descriptors) {
            for (int i = 0; i < attribute_descriptors.size(); i++) {
                auto &attrib = attribute_descriptors[i];

                if (i == 0) {
                    glBindVertexArray(attrib.vao);
                    glBindBuffer(GL_ARRAY_BUFFER, attrib.vbo);
                } else {
                    // If target VBO changed.
                    if (attrib.vbo != attribute_descriptors[i - 1].vbo) {
                        glBindBuffer(GL_ARRAY_BUFFER, attrib.vbo);
                    }
                }

                glVertexAttribIPointer(i,
                                       attrib.size,
                                       static_cast<GLenum>(attrib.type),
                                       attrib.stride,
                                       (void *) attrib.offset);

                glEnableVertexAttribArray(i);

                if (attrib.vertex_step == VertexStep::PER_VERTEX) {
                    glVertexAttribDivisor(i, 0);
                } else {
                    glVertexAttribDivisor(i, 1);
                }
            }

            check_error("bind_attributes");
        }

        static void draw(uint32_t vertex_count, uint32_t instance_count) {
            glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei) vertex_count, (GLsizei) instance_count);

            check_error("draw");
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

                Device::check_error("allocate_general_buffer");

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
