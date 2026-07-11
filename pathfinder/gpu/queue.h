#pragma once

#include "../common/math/vec2.h"
#include "command_encoder.h"
#include "fence.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

class SwapChain;

/// Handle to a command queue on a device.
class Queue {
public:
    explicit Queue() = default;

    virtual ~Queue() = default;

    virtual void submit(const std::shared_ptr<CommandEncoder> &encoder, const std::shared_ptr<Fence> &fence) = 0;
};

} // namespace Pathfinder
