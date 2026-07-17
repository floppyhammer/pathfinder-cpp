#pragma once

#include "device.h"
#include "queue.h"

namespace Pathfinder {

class OffscreenContext {
public:
    OffscreenContext(uint32_t frames_in_flight);

    virtual ~OffscreenContext() = default;

    virtual std::shared_ptr<Device> request_device() = 0;

    virtual std::shared_ptr<Queue> create_queue() = 0;

protected:
    uint32_t frames_in_flight_{};

    std::shared_ptr<Device> device_;
};

} // namespace Pathfinder
