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

struct BufferDescriptor {
    BufferType type;
    size_t size;
    MemoryProperty property;
};

class Buffer {
public:
    Buffer(const BufferDescriptor& _desc, const std::string& _label) : desc(_desc), label(_label) {}

    size_t get_size() const {
        return desc.size;
    }

    BufferType get_type() const {
        return desc.type;
    }

    MemoryProperty get_memory_property() const {
        return desc.property;
    }

    virtual void upload_via_mapping(size_t data_size, size_t offset, void* data) = 0;

    virtual void download_via_mapping(size_t data_size, size_t offset, void* data) = 0;

protected:
    BufferDescriptor desc;

    /// Debug label.
    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_H
