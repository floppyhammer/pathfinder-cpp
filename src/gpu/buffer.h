#ifndef PATHFINDER_GPU_BUFFER_H
#define PATHFINDER_GPU_BUFFER_H

#include "data.h"
#include "../common/global_macros.h"

#include <cstdint>

namespace Pathfinder {
    // Everything above 16 MB is allocated exactly for general buffer.
    const uint64_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

    class Buffer {
    public:
        Buffer(BufferType p_type, size_t p_size, BufferUsage p_usage) : type(p_type), size(p_size), usage(p_usage) {}

        inline BufferUsage get_usage() const { return usage; }

        size_t size;
        BufferType type;
        BufferUsage usage;
    };
}

#endif //PATHFINDER_GPU_BUFFER_H
