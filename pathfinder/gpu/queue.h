#pragma once

#include "command_encoder.h"
#include "fence.h"

namespace Pathfinder {

class SwapChain;

/// Handle to a command queue on a device.
class Queue {
public:
    explicit Queue() = default;

    virtual ~Queue() = default;

    virtual void submit(const std::shared_ptr<CommandEncoder> &encoder, const std::shared_ptr<Fence> &fence) = 0;

    virtual void begin_frame(uint32_t current_frame_index) {}

    virtual void wait_idle() {}

protected:
    uint32_t current_frame_index_{};
};

} // namespace Pathfinder
