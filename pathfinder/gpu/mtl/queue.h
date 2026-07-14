#pragma once

#include <Metal/Metal.h>

#include "../queue.h"

namespace Pathfinder {

class QueueMtl final : public Queue {
    friend class DeviceMtl;
    friend class WindowBuilderMtl;

public:
    void submit(const std::shared_ptr<CommandEncoder> &encoder, const std::shared_ptr<Fence> &fence) override;

    id<MTLCommandQueue> get_handle() const {
        return mtl_queue_;
    }

private:
    QueueMtl(id<MTLDevice> mtl_device, id<MTLCommandQueue> mtl_cmd_queue)
        : mtl_device_(mtl_device), mtl_queue_(mtl_cmd_queue) {}

    id<MTLCommandQueue> mtl_queue_ = nil;
    id<MTLDevice> mtl_device_ = nil;
};

} // namespace Pathfinder
