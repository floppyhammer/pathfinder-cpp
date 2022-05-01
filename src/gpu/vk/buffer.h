#ifndef PATHFINDER_GPU_BUFFER_VK_H
#define PATHFINDER_GPU_BUFFER_VK_H

#include "../buffer.h"
#include "../../common/global_macros.h"

#include <cstdint>
#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class BufferVk : public Buffer {
        friend class DriverVk;
    public:
        BufferVk(VkDevice p_device, BufferType p_type, size_t p_size);

        ~BufferVk();

        VkBuffer get_vk_buffer();

    private:
        VkBuffer vk_buffer{};
        VkDeviceMemory vk_device_memory{};

        VkDevice vk_device;
    };
}

#endif

#endif //PATHFINDER_GPU_BUFFER_VK_H
