#ifndef PATHFINDER_GPU_COMMAND_BUFFER_VK_H
#define PATHFINDER_GPU_COMMAND_BUFFER_VK_H

#include "../command_encoder.h"
#include "device.h"

namespace Pathfinder {

class CommandEncoderVk : public CommandEncoder {
    friend class DeviceVk;
    friend class QueueVk;
    friend class SwapChainVk;

public:
    ~CommandEncoderVk() override;

private:
    CommandEncoderVk(VkCommandBuffer vk_command_buffer, DeviceVk *device);

    void sync_descriptor_set(DescriptorSet *descriptor_set);

    bool finish() override;

    VkCommandBuffer vk_command_buffer_{};

    VkDevice vk_device_{};

    DeviceVk *device_vk_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_COMMAND_BUFFER_VK_H
