//
// Created by floppyhammer on 2021/12/31.
//

#include "device.h"

#include "../common/logger.h"

#include <iostream>
#include <sstream>

namespace Pathfinder {
    std::shared_ptr<Buffer> Device::create_buffer(BufferType type, size_t size) {
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

    void Device::upload_to_buffer(const std::shared_ptr<Buffer>& buffer, size_t offset, size_t data_size, void *data) {
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

    void Device::check_error(const char *flag) {
#ifdef PATHFINDER_DEBUG
        for (GLint error = glGetError(); error; error = glGetError()) {
            std::ostringstream string_stream;
            string_stream << "Error " << error << " after " << flag;
            Logger::error(string_stream.str(), "OpenGL");
        }
#endif
    }

    void Device::print_string(const char *name, GLenum s) {
#ifdef PATHFINDER_DEBUG
        const char *v = (const char *) glGetString(s);

        std::ostringstream string_stream;
        string_stream << "GL " << name << " = " << v;

        Logger::error(string_stream.str(), "OpenGL");
#endif
    }
}
