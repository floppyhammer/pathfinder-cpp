#include "buffer.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include "driver.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

BufferVk::BufferVk(VkDevice _vk_device, const BufferDescriptor& _desc) : Buffer(_desc), vk_device(_vk_device) {}

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

void BufferVk::upload_via_mapping(size_t data_size, size_t offset, void* data) {
    if (desc.property != MemoryProperty::HostVisibleAndCoherent) {
        abort();
    }

    void* mapped_data;
    auto res = vkMapMemory(vk_device, vk_device_memory, offset, data_size, 0, &mapped_data);
    memcpy(mapped_data, data, data_size);
    vkUnmapMemory(vk_device, vk_device_memory);

    if (res != VK_SUCCESS) {
        Logger::error("Failed to map memory!", "BufferVk");
    }
}

void BufferVk::download_via_mapping(size_t data_size, size_t offset, void* data) {
    if (desc.property != MemoryProperty::HostVisibleAndCoherent) {
        abort();
    }

    void* mapped_data;
    auto res = vkMapMemory(vk_device, vk_device_memory, offset, data_size, 0, &mapped_data);
    memcpy(data, mapped_data, data_size);
    vkUnmapMemory(vk_device, vk_device_memory);

    if (res != VK_SUCCESS) {
        Logger::error("Failed to map memory!", "BufferVk");
    }
}

} // namespace Pathfinder

#endif
