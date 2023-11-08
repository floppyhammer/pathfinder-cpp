#ifndef PATHFINDER_GPU_BUFFER_GL_H
#define PATHFINDER_GPU_BUFFER_GL_H

#include <cstdint>

#include "../../common/global_macros.h"
#include "../buffer.h"

namespace Pathfinder {

class BufferGl : public Buffer {
    friend class DeviceGl;

public:
    ~BufferGl() override;

    void upload_via_mapping(size_t data_size, size_t offset, const void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    uint32_t get_handle() const;

    void set_label(const std::string& _label) override;

private:
    explicit BufferGl(const BufferDescriptor& _desc);

private:
    uint32_t id = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_GL_H
