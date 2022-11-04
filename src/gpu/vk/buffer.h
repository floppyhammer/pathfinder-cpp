#ifndef PATHFINDER_GPU_BUFFER_VK_H
#define PATHFINDER_GPU_BUFFER_VK_H

#include <cstdint>
#include <memory>

#include "../../common/global_macros.h"
#include "../buffer.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class BufferVk : public Buffer {
    friend class DriverVk;

public:
    BufferVk(VkDevice _device, BufferType _type, size_t _size, MemoryProperty _memory_property);

    ~BufferVk();

    VkBuffer get_vk_buffer();

    VkDeviceMemory get_vk_device_memory();

private:
    VkBuffer vk_buffer{};
    VkDeviceMemory vk_device_memory{};

    VkDevice device;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_BUFFER_VK_H
