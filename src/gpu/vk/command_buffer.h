#ifndef PATHFINDER_GPU_COMMAND_BUFFER_VK_H
#define PATHFINDER_GPU_COMMAND_BUFFER_VK_H

#include <cstdint>
#include <memory>
#include <queue>

#include "../command_buffer.h"
#include "device.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class CommandBufferVk : public CommandBuffer {
    friend class DeviceVk;

    friend class SwapChainVk;

public:
    void finish() override;

    /// Submit and wait for implementation to finish. At last, free the command buffer.
    void submit_and_wait() override;

    VkCommandBuffer get_vk_handle() const;

private:
    CommandBufferVk(VkCommandBuffer _vk_command_buffer, DeviceVk *_device);

    void sync_descriptor_set(DescriptorSet *descriptor_set);

private:
    VkCommandBuffer vk_command_buffer{};

    VkDevice vk_device{};

    DeviceVk *device_vk{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMMAND_BUFFER_VK_H
