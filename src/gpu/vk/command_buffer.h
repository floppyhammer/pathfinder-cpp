#ifndef PATHFINDER_GPU_COMMAND_BUFFER_VK_H
#define PATHFINDER_GPU_COMMAND_BUFFER_VK_H

#include "../command_buffer.h"

#include <cstdint>
#include <queue>
#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class CommandBufferVk : public CommandBuffer {
        friend class DriverVk;

        friend class SwapChainVk;

    public:
        /// We have to provide these two to create a valid command buffer.
        CommandBufferVk(VkCommandBuffer p_command_buffer, VkDevice p_device);

        void upload_to_buffer(const std::shared_ptr<Buffer> &buffer,
                              uint32_t offset,
                              uint32_t data_size,
                              void *data) override;

        void submit(const std::shared_ptr<Driver> &p_driver) override;

    private:
        VkCommandBuffer vk_command_buffer;

        VkDevice device;
    };
}

#endif

#endif //PATHFINDER_GPU_COMMAND_BUFFER_VK_H
