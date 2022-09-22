#ifndef PATHFINDER_GPU_BUFFER_H
#define PATHFINDER_GPU_BUFFER_H

#include <cstdint>

#include "../common/global_macros.h"
#include "data.h"

namespace Pathfinder {
// Everything above 16 MB is allocated exactly for general buffer.
const uint64_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

// Maximum binding number of vertex buffers during a draw call.
const uint32_t MAX_VERTEX_BUFFER_BINDINGS = 8;

class Buffer {
public:
    Buffer(BufferType p_type, size_t p_size, MemoryProperty p_property)
        : type(p_type), size(p_size), memory_property(p_property) {
    }

    MemoryProperty get_memory_property() const {
        return memory_property;
    }

    size_t size;

    BufferType type;

    MemoryProperty memory_property;
};
} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_H
