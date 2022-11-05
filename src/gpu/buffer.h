#ifndef PATHFINDER_GPU_BUFFER_H
#define PATHFINDER_GPU_BUFFER_H

#include <cstdint>
#include <string>
#include <utility>

#include "../common/global_macros.h"
#include "data.h"

namespace Pathfinder {

// Everything above 16 MB is allocated exactly for general buffer.
const uint64_t MAX_BUFFER_SIZE_CLASS = 16 * 1024 * 1024;

// Maximum binding number of vertex buffers during a draw call.
const uint32_t MAX_VERTEX_BUFFER_BINDINGS = 8;

class Buffer {
public:
    Buffer(BufferType _type, size_t _size, MemoryProperty _memory_property, std::string _label)
        : type(_type), size(_size), memory_property(_memory_property), label(std::move(_label)) {}

    size_t get_size() const {
        return size;
    }

    BufferType get_type() const {
        return type;
    }

    MemoryProperty get_memory_property() const {
        return memory_property;
    }

protected:
    size_t size;

    BufferType type;

    MemoryProperty memory_property;

    /// Debug label.
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_H
