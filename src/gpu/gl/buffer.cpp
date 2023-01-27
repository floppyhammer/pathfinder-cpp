#include "buffer.h"

#include "../../common/logger.h"
#include "../../common/math/basic.h"
#include "validation.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

BufferGl::BufferGl(const BufferDescriptor &_desc) : Buffer(_desc) {
    if (_desc.size == 0) {
        Logger::error("Attempted to create a buffer of zero size!");
    }

    glGenBuffers(1, &id);

    GLint target = GL_NONE;

    switch (_desc.type) {
        case BufferType::Uniform: {
            target = GL_UNIFORM_BUFFER;
        } break;
        case BufferType::Vertex: {
            target = GL_ARRAY_BUFFER;
        } break;
        case BufferType::Index: {
            Logger::error("Cannot handle index buffers yet!");
            abort();
        } break;
        case BufferType::Storage: {
    #ifdef PATHFINDER_USE_D3D11
            target = GL_SHADER_STORAGE_BUFFER;
    #endif
        } break;
    }

    glBindBuffer(target, id);
    glBufferData(target, _desc.size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(target, 0);

    gl_check_error("create_buffer");

    DebugMarker::label_buffer(id, _desc.label);
}

BufferGl::~BufferGl() {
    glDeleteBuffers(1, &id);
}

void BufferGl::upload_via_mapping(size_t data_size, size_t offset, void *data) {
    int gl_buffer_type = 0;

    switch (desc.type) {
        case BufferType::Vertex: {
            gl_buffer_type = GL_ARRAY_BUFFER;
        } break;
        case BufferType::Uniform: {
            gl_buffer_type = GL_UNIFORM_BUFFER;
        } break;
        case BufferType::Index: {
            Logger::error("Cannot handle index buffers yet!");
        } break;
        case BufferType::Storage: {
    #ifdef PATHFINDER_USE_D3D11
            gl_buffer_type = GL_SHADER_STORAGE_BUFFER;
    #endif
        } break;
    }

    glBindBuffer(gl_buffer_type, id);
    glBufferSubData(gl_buffer_type, (GLintptr)offset, (GLsizeiptr)data_size, data);
    glBindBuffer(gl_buffer_type, 0);

    gl_check_error("UploadToBuffer");
}

void BufferGl::download_via_mapping(size_t data_size, size_t offset, void *data) {
    // We can only read from storage buffers.
    if (desc.type != BufferType::Storage) {
        Logger::error("Tried to read from a non-storage buffer!", "BufferGl");
        return;
    }

    #ifdef PATHFINDER_USE_D3D11
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        #ifdef __ANDROID__
    void *ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, args.offset, args.data_size, GL_MAP_READ_BIT);
    if (ptr) {
        memcpy(args.data, ptr, args.data_size);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        #else
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, data_size, data);
        #endif
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind.
    #endif

    gl_check_error("ReadBuffer");
}

uint32_t BufferGl::get_handle() const {
    return id;
}

} // namespace Pathfinder

#endif
