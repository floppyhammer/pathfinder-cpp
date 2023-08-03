#ifndef PATHFINDER_GPU_COMMAND_BUFFER_GL_H
#define PATHFINDER_GPU_COMMAND_BUFFER_GL_H

#include <cstdint>
#include <memory>
#include <queue>

#include "../command_buffer.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class CommandBufferGl : public CommandBuffer {
    friend class DeviceGl;
    friend class SwapChainGl;

public:
    void finish() override;

    void submit_and_wait() override;

private:
    CommandBufferGl() = default;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMMAND_BUFFER_GL_H
