#ifndef PATHFINDER_GPU_COMMAND_BUFFER_VK_H
#define PATHFINDER_GPU_COMMAND_BUFFER_VK_H

#include <cstdint>
#include <memory>
#include <queue>

#include "../command_buffer.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class CommandBufferVk : public CommandBuffer {
    friend class DriverVk;

    friend class SwapChainVk;

public:
    /// We have to provide these two to create a valid command buffer.
    CommandBufferVk(VkCommandBuffer _vk_command_buffer, VkDevice _vk_device, DriverVk *_driver);

    ~CommandBufferVk();

    void upload_to_buffer(const std::shared_ptr<Buffer> &buffer,
                          uint32_t offset,
                          uint32_t data_size,
                          void *data) override;

    void submit() override;

    void submit_and_wait() override;

    VkCommandBuffer get_vk_command_buffer() const;

private:
    VkCommandBuffer vk_command_buffer{};

    VkDevice vk_device{};

    DriverVk *driver;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_COMMAND_BUFFER_VK_H
