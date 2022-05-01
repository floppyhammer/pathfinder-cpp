#include "buffer.h"

#include "driver.h"

#include <stdexcept>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    BufferVk::BufferVk(VkDevice p_device, BufferType p_type, size_t p_size, BufferUsage usage)
            : Buffer(p_type, p_size, usage), vk_device(p_device) {}

    BufferVk::~BufferVk() {
        vkDestroyBuffer(vk_device, vk_buffer, nullptr);
        vkFreeMemory(vk_device, vk_device_memory, nullptr);
    }

    VkBuffer BufferVk::get_vk_buffer() {
        return vk_buffer;
    }

    VkDeviceMemory BufferVk::get_vk_device_memory() {
        return vk_device_memory;
    }
}

#endif
