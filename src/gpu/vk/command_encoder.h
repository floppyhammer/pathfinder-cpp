#ifndef PATHFINDER_GPU_COMMAND_BUFFER_VK_H
#define PATHFINDER_GPU_COMMAND_BUFFER_VK_H

#include <cstdint>
#include <memory>
#include <queue>

#include "../command_encoder.h"
#include "device.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class CommandEncoderVk : public CommandEncoder {
    friend class DeviceVk;
    friend class QueueVk;
    friend class SwapChainVk;

public:
    VkCommandBuffer get_vk_handle() const;

private:
    CommandEncoderVk(VkCommandBuffer _vk_command_buffer, DeviceVk *_device);

    ~CommandEncoderVk() override;

    void sync_descriptor_set(DescriptorSet *descriptor_set);

    void finish() override;

private:
    VkCommandBuffer vk_command_buffer{};

    VkDevice vk_device{};

    DeviceVk *device_vk{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMMAND_BUFFER_VK_H
