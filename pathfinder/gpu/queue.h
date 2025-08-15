#ifndef PATHFINDER_GPU_QUEUE_H
#define PATHFINDER_GPU_QUEUE_H

#include "../common/math/vec2.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

class SwapChain;

/// Handle to a command queue on a device.
class Queue {
public:
    explicit Queue() = default;

    virtual ~Queue() = default;

    virtual void submit_and_wait(const std::shared_ptr<CommandEncoder> &encoder) = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_QUEUE_H
