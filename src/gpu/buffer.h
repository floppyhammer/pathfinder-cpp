#ifndef PATHFINDER_GPU_BUFFER_H
#define PATHFINDER_GPU_BUFFER_H

#include <cstdint>
#include <string>
#include <utility>

#include "../common/global_macros.h"
#include "data.h"

namespace Pathfinder {

// Maximum binding number of vertex buffers during a draw call.
const uint32_t MAX_VERTEX_BUFFER_BINDINGS = 8;

struct BufferDescriptor {
    BufferType type;
    size_t size;
    MemoryProperty property;
    std::string label;
};

class Buffer {
public:
    explicit Buffer(BufferDescriptor _desc) : desc(std::move(_desc)) {}

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
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_H
