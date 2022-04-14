//
// Created by floppyhammer on 2021/12/31.
//

#include "device.h"
#include "validation.h"

#include "../common/logger.h"

namespace Pathfinder {
    std::shared_ptr<Buffer> Device::create_buffer(BufferType type, size_t size) {
        if (size == 0) {
            Logger::error("Tried to create a buffer with zero size!");
        }

        auto buffer = std::make_shared<Buffer>();

        switch (type) {
            case BufferType::Uniform: {
                unsigned int buffer_id;
                glGenBuffers(1, &buffer_id);
                glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
                glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

                buffer->type = BufferType::Uniform;
                buffer->size = size;
                buffer->id = buffer_id;
            } break;
            case BufferType::Vertex: {
                unsigned int buffer_id;
                glGenBuffers(1, &buffer_id);
                glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
                glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

                buffer->type = BufferType::Vertex;
                buffer->size = size;
                buffer->id = buffer_id;
            }
                break;
#ifdef PATHFINDER_USE_D3D11
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
                buffer->id = buffer_id;
            }
                break;
#endif
        }

        check_error("create_buffer");
        return buffer;
    }

    std::shared_ptr<Texture> Device::create_texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type) {
        auto texture = std::make_shared<Texture>(p_width, p_height, p_format, p_type);

        check_error("create_texture");

        return texture;
    }

    std::shared_ptr<CommandBuffer> Device::create_command_buffer() {
        return std::make_shared<CommandBuffer>();
    }
}
