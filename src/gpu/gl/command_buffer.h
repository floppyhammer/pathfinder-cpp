#ifndef PATHFINDER_GPU_COMMAND_BUFFER_GL_H
#define PATHFINDER_GPU_COMMAND_BUFFER_GL_H

#include <cstdint>
#include <memory>
#include <queue>

#include "../command_buffer.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class CommandBufferGl : public CommandBuffer {
public:
    void upload_to_buffer(const std::shared_ptr<Buffer> &buffer,
                          uint32_t offset,
                          uint32_t data_size,
                          void *data) override;

    void submit(const std::shared_ptr<Driver> &_driver) override;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMMAND_BUFFER_GL_H
