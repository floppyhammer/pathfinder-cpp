#ifndef PATHFINDER_BUFFER_VK_H
#define PATHFINDER_BUFFER_VK_H

#include "../buffer.h"
#include "../platform.h"
#include "../../common/global_macros.h"

#include <cstdint>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class BufferVk : Buffer {
    public:
        BufferVk(BufferType p_type, size_t p_size);

        ~BufferVk();

    private:
        VkBuffer id{};
        VkDeviceMemory device_memory{};
    };
}

#endif

#endif //PATHFINDER_BUFFER_VK_H
