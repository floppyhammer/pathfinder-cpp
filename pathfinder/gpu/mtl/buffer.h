#pragma once

#include <memory>

#include "../buffer.h"
#include "device.h"

namespace Pathfinder {

class BufferMtl final : public Buffer {
    friend class DeviceMtl;
    friend class CommandEncoderMtl;

public:
    void upload_via_mapping(size_t data_size, size_t offset, const void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    void set_label(const std::string& label) override;

    id<MTLBuffer> get_handle() noexcept {
        return mtl_buffer_;
    }

private:
    BufferMtl(const BufferDescriptor& descriptor);

    id<MTLBuffer> mtl_buffer_ = nil;
};

} // namespace Pathfinder
