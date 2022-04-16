#ifndef PATHFINDER_BUFFER_VK_H
#define PATHFINDER_BUFFER_VK_H

#include "../gl/buffer.h"
#include "../../common/global_macros.h"
#include "../platform.h"

#include <cstdint>

namespace Pathfinder {
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        auto device = dynamic_cast<DeviceVk *>(Platform::get_singleton().device.get())->get_device();

        // Structure specifying the parameters of a newly created buffer object.
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size; // Size in bytes of the buffer to be created.
        bufferInfo.usage = usage; // Specifying allowed usages of the buffer.
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Specifying the sharing mode of the buffer when it will be accessed by multiple queue families.

        // Allocate GPU buffer.
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        // Structure containing parameters of a memory allocation.
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                                   properties); // Index identifying a memory type.

        // Allocate CPU buffer memory.
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        // Bind GPU buffer and CPU buffer memory.
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    class BufferVk : Buffer {
    public:
        BufferVk(BufferType p_type, size_t p_size) : Buffer(p_type, p_size) {
            createBuffer(p_size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         id,
                         device_memory);
        }

        ~BufferVk() {
            auto device = dynamic_cast<DeviceVk *>(Platform::get_singleton().device.get())->get_device();

            vkDestroyBuffer(device, id, nullptr);
            vkFreeMemory(device, device_memory, nullptr);
        };

    private:
        VkBuffer id{};
        VkDeviceMemory device_memory{};
    };
}

#endif //PATHFINDER_BUFFER_VK_H
