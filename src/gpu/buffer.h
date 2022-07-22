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
        Buffer(BufferType p_type, size_t p_size, MemoryProperty p_property)
                : type(p_type), size(p_size), memory_property(p_property) {}

        inline MemoryProperty get_memory_property() const { return memory_property; }

        /// Buffer size.
        size_t size;

        BufferType type;

        /// Device local or shared between host and device.
        MemoryProperty memory_property;
    };
}

#endif //PATHFINDER_GPU_BUFFER_H
