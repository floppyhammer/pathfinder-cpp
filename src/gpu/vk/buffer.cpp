#include "buffer.h"

#include "driver.h"

#include <stdexcept>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    BufferVk::BufferVk(VkDevice p_device, BufferType p_type, size_t p_size)
            : Buffer(p_type, p_size), device(p_device) {}

    BufferVk::~BufferVk() {
        vkDestroyBuffer(device, id, nullptr);
        vkFreeMemory(device, device_memory, nullptr);
    }
}

#endif
