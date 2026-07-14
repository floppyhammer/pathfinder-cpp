#include "buffer.h"

#include "base.h"
#include "command_encoder.h"

namespace Pathfinder {

BufferMtl::BufferMtl(const BufferDescriptor& descriptor) : Buffer(descriptor) {}

void BufferMtl::upload_via_mapping(size_t data_size, size_t offset, const void* data) {
    unsigned char* data_ptr = reinterpret_cast<unsigned char*>([mtl_buffer_ contents]) + offset;
    memcpy(data_ptr, data, data_size);
}

void BufferMtl::download_via_mapping(size_t data_size, size_t offset, void* data) {
    unsigned char* data_ptr = reinterpret_cast<unsigned char*>([mtl_buffer_ contents]) + offset;
    memcpy(data, data_ptr, data_size);
}

void BufferMtl::set_label(const std::string& label) {
    Buffer::set_label(label);
}

} // namespace Pathfinder
