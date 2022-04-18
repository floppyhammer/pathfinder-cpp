#ifndef PATHFINDER_HAL_BUFFER_H
#define PATHFINDER_HAL_BUFFER_H

#include "../common/global_macros.h"

#include <cstdint>

namespace Pathfinder {
    // Everything above 16 MB is allocated exactly for general buffer.
    const uint64_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

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
        Buffer(BufferType p_type, size_t p_size) : type(p_type), size(p_size) {}

        size_t size;
        BufferType type;
    };
}

#endif //PATHFINDER_HAL_BUFFER_H
