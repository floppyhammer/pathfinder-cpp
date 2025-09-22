#ifndef PATHFINDER_GPU_FENCE_VK_H
#define PATHFINDER_GPU_FENCE_VK_H

#include "../fence.h"
#include "base.h"

namespace Pathfinder {

class DeviceVk;

class FenceVk : public Fence {
    friend class DeviceVk;
    friend class QueueVk;

public:
    ~FenceVk();

    void wait() const;

private:
    FenceVk() = default;

    VkFence fence = VK_NULL_HANDLE;
    DeviceVk *device = nullptr;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_FENCE_VK_H
