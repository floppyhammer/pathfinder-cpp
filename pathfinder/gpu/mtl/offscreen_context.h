#pragma once

#include "../offscreen_context.h"

namespace Pathfinder {

class OffscreenContextMtl : public OffscreenContext {
public:
    /**
     * @param device Pointer to id<MTLDevice>. Note: The caller must ensure the
     *               underlying Objective-C object outlives this context.
     * @param mtl_cmd_queue Pointer to id<MTLCommandQueue>.
     */
    OffscreenContextMtl(void* device, void* mtl_cmd_queue, uint32_t frames_in_flight);

    ~OffscreenContextMtl() override;

    std::shared_ptr<Device> request_device() override;

    std::shared_ptr<Queue> create_queue() override;

private:
    void* mtl_device_{};
    void* mtl_cmd_queue_{};
};

} // namespace Pathfinder
