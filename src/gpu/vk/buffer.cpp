#include "buffer.h"

#include <stdexcept>

#include "driver.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
BufferVk::BufferVk(VkDevice p_device, BufferType p_type, size_t p_size, MemoryProperty property)
    : Buffer(p_type, p_size, property), device(p_device) {
}

BufferVk::~BufferVk() {
    vkDestroyBuffer(device, vk_buffer, nullptr);
    vkFreeMemory(device, vk_device_memory, nullptr);
}

VkBuffer BufferVk::get_vk_buffer() {
    return vk_buffer;
}

VkDeviceMemory BufferVk::get_vk_device_memory() {
    return vk_device_memory;
}
} // namespace Pathfinder

#endif
