#ifndef PATHFINDER_HAL_BUFFER_VK_H
#define PATHFINDER_HAL_BUFFER_VK_H

#include "../buffer.h"
#include "../../common/global_macros.h"

#include <cstdint>
#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class BufferVk : public Buffer {
    public:
        BufferVk(VkDevice p_device, BufferType p_type, size_t p_size);

        ~BufferVk();

    private:
        VkBuffer id{};
        VkDeviceMemory device_memory{};

        VkDevice device;

        friend class DeviceVk;
    };
}

#endif

#endif //PATHFINDER_HAL_BUFFER_VK_H
