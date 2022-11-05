#include "buffer.h"

#include <stdexcept>
#include <utility>

#include "driver.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

BufferVk::BufferVk(VkDevice _vk_device,
                   BufferType _type,
                   size_t _size,
                   MemoryProperty _memory_property,
                   std::string _label)
    : Buffer(_type, _size, _memory_property, std::move(_label)), vk_device(_vk_device) {}

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

} // namespace Pathfinder

#endif
