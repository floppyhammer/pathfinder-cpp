#pragma once

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

    void add_barriers_for_descriptor_set(DescriptorSet *descriptor_set);

    bool prepare() override;

    VkCommandBuffer vk_command_buffer_{};

    VkDevice vk_device_{};

    DeviceVk *device_vk_{};
};

} // namespace Pathfinder
