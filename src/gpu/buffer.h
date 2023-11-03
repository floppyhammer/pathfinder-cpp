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

    inline bool operator==(const BufferDescriptor& b) const {
        return size == b.size && type == b.type && property == b.property;
    }
};

class Buffer {
public:
    explicit Buffer(BufferDescriptor _desc) : desc(_desc) {}

    virtual ~Buffer() = default;

    size_t get_size() const {
        return desc.size;
    }

    BufferType get_type() const {
        return desc.type;
    }

    MemoryProperty get_memory_property() const {
        return desc.property;
    }

    virtual void upload_via_mapping(size_t data_size, size_t offset, const void* data) = 0;

    virtual void download_via_mapping(size_t data_size, size_t offset, void* data) = 0;

    // Sometimes, we need to update label for a buffer as we reuse it for another purpose.
    virtual void set_label(const std::string& _label){};

protected:
    BufferDescriptor desc;

    std::string label;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_H
