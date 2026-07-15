#pragma once

#include <cstdint>

#include "../buffer.h"

namespace Pathfinder {

class BufferGl : public Buffer {
    friend class DeviceGl;

public:
    ~BufferGl() override;

    void upload_via_mapping(size_t data_size, size_t offset, const void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    void* map();

    void unmap();

    uint32_t get_handle() const;

    void set_label(const std::string& label) override;

private:
    explicit BufferGl(const BufferDescriptor& desc);
    uint32_t gl_id_ = 0;

    // Cache mapped pointer to improve performance.
    void* mapped_ptr_ = nullptr;
};

} // namespace Pathfinder
