#ifndef PATHFINDER_GPU_BUFFER_H
#define PATHFINDER_GPU_BUFFER_H

#include <cstdint>
#include <string>

#include "base.h"

namespace Pathfinder {

// Maximum binding number of vertex buffers for a draw call.
const uint32_t MAX_VERTEX_BUFFER_BINDINGS = 8;

struct BufferDescriptor {
    BufferType type;
    size_t size;
    MemoryProperty property;

    bool operator==(const BufferDescriptor& b) const {
        return size == b.size && type == b.type && property == b.property;
    }
};

class Buffer {
public:
    explicit Buffer(const BufferDescriptor& desc) : desc_(desc) {}

    virtual ~Buffer() = default;

    size_t get_size() const {
        return desc_.size;
    }

    BufferType get_type() const {
        return desc_.type;
    }

    MemoryProperty get_memory_property() const {
        return desc_.property;
    }

    virtual void upload_via_mapping(size_t data_size, size_t offset, const void* data) = 0;

    virtual void download_via_mapping(size_t data_size, size_t offset, void* data) = 0;

    // Sometimes, we need to update label for a buffer as we reuse it for another purpose.
    virtual void set_label(const std::string& label) {
        label_ = label;
    }

protected:
    BufferDescriptor desc_;

    std::string label_;
};

} // namespace Pathfinder

#endif
