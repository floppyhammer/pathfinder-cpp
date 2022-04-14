//
// Created by floppyhammer on 4/9/2022.
//

#ifndef PATHFINDER_BUFFER_H
#define PATHFINDER_BUFFER_H

#include "../common/global_macros.h"

#include <cstdint>

namespace Pathfinder {
    enum class BufferType {
        Vertex,
        Uniform,
        General,
    };

    enum class GeneralBufferUsage {
        Read,
        Write,
        ReadWrite,
    };

    class Buffer {
    public:
        ~Buffer() {
            glDeleteBuffers(1, &id);
        };

        uint32_t id;
        size_t size;
        BufferType type;
    };
}

#endif //PATHFINDER_BUFFER_H
