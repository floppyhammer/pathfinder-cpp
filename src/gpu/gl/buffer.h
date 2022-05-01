#ifndef PATHFINDER_GPU_BUFFER_GL_H
#define PATHFINDER_GPU_BUFFER_GL_H

#include "../buffer.h"
#include "../../common/global_macros.h"

#include <cstdint>

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class BufferGl : public Buffer {
    public:
        BufferGl(BufferType p_type, size_t p_size);

        ~BufferGl() {
            glDeleteBuffers(1, &id);
        };

        uint32_t id;
    };
}

#endif

#endif //PATHFINDER_GPU_BUFFER_GL_H
