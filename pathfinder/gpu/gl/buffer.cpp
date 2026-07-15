#include "buffer.h"

#include <cassert>
#include <cstring>

#include "../../common/logger.h"
#include "debug_marker.h"

namespace Pathfinder {

BufferGl::BufferGl(const BufferDescriptor &desc) : Buffer(desc) {
    if (desc.size == 0) {
        Logger::error("Attempted to create a buffer of zero size!");
    }

    glGenBuffers(1, &gl_id_);
    assert(gl_id_ != 0);

    GLint target = GL_NONE;

    switch (desc.type) {
        case BufferType::Uniform: {
            target = GL_UNIFORM_BUFFER;
        } break;
        case BufferType::Vertex: {
            target = GL_ARRAY_BUFFER;
        } break;
        case BufferType::Index: {
            target = GL_ELEMENT_ARRAY_BUFFER;
        } break;
        case BufferType::Storage: {
#ifdef PATHFINDER_ENABLE_COMPUTE
            target = GL_SHADER_STORAGE_BUFFER;
#else
            target = GL_COPY_WRITE_BUFFER;
#endif
        } break;
    }

    glBindBuffer(target, gl_id_);
    glBufferData(target, desc.size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(target, 0);

    gl_check_error("create_buffer");
}

BufferGl::~BufferGl() {
    glDeleteBuffers(1, &gl_id_);
}

void BufferGl::upload_via_mapping(size_t data_size, size_t offset, const void *data) {
    int gl_buffer_type = 0;

    switch (desc_.type) {
        case BufferType::Vertex: {
            gl_buffer_type = GL_ARRAY_BUFFER;
        } break;
        case BufferType::Uniform: {
            gl_buffer_type = GL_UNIFORM_BUFFER;
        } break;
        case BufferType::Index: {
            gl_buffer_type = GL_ELEMENT_ARRAY_BUFFER;
        } break;
        case BufferType::Storage: {
#ifdef PATHFINDER_ENABLE_COMPUTE
            gl_buffer_type = GL_SHADER_STORAGE_BUFFER;
#else
            gl_buffer_type = GL_COPY_WRITE_BUFFER;
#endif
        } break;
    }

    glBindBuffer(gl_buffer_type, gl_id_);
    glBufferSubData(gl_buffer_type, (GLintptr)offset, (GLsizeiptr)data_size, data);
    glBindBuffer(gl_buffer_type, 0);

    gl_check_error("UploadToBuffer");
}

void BufferGl::download_via_mapping(size_t data_size, size_t offset, void *data) {
    if (desc_.type != BufferType::Storage) {
        Logger::error("Tried to read from a non-storage buffer!");
        return;
    }

#ifdef PATHFINDER_ENABLE_COMPUTE
    GLenum target = GL_SHADER_STORAGE_BUFFER;
#else
    GLenum target = GL_COPY_READ_BUFFER;
#endif

    glBindBuffer(target, gl_id_);
#if defined(__ANDROID__) || (defined(__linux__) && defined(__ARM_ARCH)) || (defined(_WIN32) && defined(_M_ARM64))
    void *ptr = glMapBufferRange(target, offset, data_size, GL_MAP_READ_BIT);
    if (ptr) {
        memcpy(data, ptr, data_size);
    }
    glUnmapBuffer(target);
#else
    glGetBufferSubData(target, offset, data_size, data);
#endif
    glBindBuffer(target, 0);

    gl_check_error("ReadBuffer");
}

uint32_t BufferGl::get_handle() const {
    return gl_id_;
}

void BufferGl::set_label(const std::string &label) {
    Buffer::set_label(label);

    DebugMarker::label_buffer(gl_id_, label);
}

void *BufferGl::map() {
    GLint target = GL_NONE;
    switch (desc_.type) {
        case BufferType::Uniform:
            target = GL_UNIFORM_BUFFER;
            break;
        case BufferType::Vertex:
            target = GL_ARRAY_BUFFER;
            break;
        case BufferType::Index:
            target = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case BufferType::Storage: {
#ifdef PATHFINDER_ENABLE_COMPUTE
            target = GL_SHADER_STORAGE_BUFFER;
#else
            target = GL_COPY_WRITE_BUFFER;
#endif
        } break;
    }
    glBindBuffer(target, gl_id_);
    void *ptr = glMapBufferRange(target, 0, desc_.size, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    glBindBuffer(target, 0);
    return ptr;
}

void BufferGl::unmap() {
    GLint target = GL_NONE;
    switch (desc_.type) {
        case BufferType::Uniform:
            target = GL_UNIFORM_BUFFER;
            break;
        case BufferType::Vertex:
            target = GL_ARRAY_BUFFER;
            break;
        case BufferType::Index:
            target = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case BufferType::Storage: {
#ifdef PATHFINDER_ENABLE_COMPUTE
            target = GL_SHADER_STORAGE_BUFFER;
#else
            target = GL_COPY_WRITE_BUFFER;
#endif
        } break;
    }
    glBindBuffer(target, gl_id_);
    glUnmapBuffer(target);
    glBindBuffer(target, 0);
}

} // namespace Pathfinder
