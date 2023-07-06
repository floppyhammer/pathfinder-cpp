#ifndef PATHFINDER_GPU_BUFFER_GL_H
#define PATHFINDER_GPU_BUFFER_GL_H

#include <cstdint>

#include "../../common/global_macros.h"
#include "../buffer.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class BufferGl : public Buffer {
public:
    explicit BufferGl(const BufferDescriptor& _desc);

    ~BufferGl();

    void upload_via_mapping(size_t data_size, size_t offset, void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    uint32_t get_handle() const;

    void set_label(const std::string& _label) override;

private:
    uint32_t id = 0;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_BUFFER_GL_H
