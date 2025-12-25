#include "fence.h"

#include "device.h"

namespace Pathfinder {

FenceVk::~FenceVk() {
    if (fence) {
        vkDestroyFence(device->get_device(), fence, nullptr);
    }
}

void FenceVk::wait() const {
    // Wait indefinitely.
    vkWaitForFences(device->get_device(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->get_device(), 1, &fence);
}

} // namespace Pathfinder
