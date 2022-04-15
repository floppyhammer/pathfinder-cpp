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
        size_t size;
        BufferType type;
    };
}

#endif //PATHFINDER_BUFFER_H
