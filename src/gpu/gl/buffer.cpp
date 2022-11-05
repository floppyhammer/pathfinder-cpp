#include "buffer.h"

#include "../../common/logger.h"
#include "../../common/math/basic.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

BufferGl::BufferGl(BufferType _type, size_t _size, MemoryProperty _memory_property, std::string _label)
    : Buffer(_type, _size, _memory_property, std::move(_label)) {
    if (size == 0) {
        Logger::error("Tried to create a buffer with zero size!");
    }

    glGenBuffers(1, &id);

    switch (type) {
        case BufferType::Uniform: {
            glBindBuffer(GL_UNIFORM_BUFFER, id);
            glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0); // Unbind.
        } break;
        case BufferType::Vertex: {
            glBindBuffer(GL_ARRAY_BUFFER, id);
            glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind.
        } break;
        case BufferType::Storage: {
    #ifdef PATHFINDER_USE_D3D11
            if (size < MAX_BUFFER_SIZE_CLASS) {
                size = upper_power_of_two(size);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
            glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.
    #endif
        } break;
    }
}

BufferGl::~BufferGl() {
    glDeleteBuffers(1, &id);
}

} // namespace Pathfinder

#endif
