#include "buffer.h"

#include <stdexcept>

#include "driver.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

BufferVk::BufferVk(VkDevice _device, BufferType _type, size_t _size, MemoryProperty _memory_property)
    : Buffer(_type, _size, _memory_property), device(_device) {}

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
