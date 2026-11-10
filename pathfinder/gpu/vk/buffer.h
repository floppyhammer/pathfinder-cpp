#pragma once

#include "../buffer.h"
#include "base.h"
#include "vma_header.h"

namespace Pathfinder {

class DeviceVk;

class BufferVk : public Buffer {
    friend class DeviceVk;
    friend class CommandEncoderVk;

public:
    ~BufferVk() override;

    void upload_via_mapping(size_t data_size, size_t offset, const void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    VkBuffer get_vk_buffer();

    VkDeviceMemory get_vk_device_memory();

    VmaAllocation get_vma_allocation() const;

    /// Map the whole buffer once, offset is zero.
    void* map();

    void unmap();

private:
    BufferVk(VmaAllocator vma_allocator, VkDevice vk_device, const BufferDescriptor& desc);

    VkBuffer vk_buffer_{VK_NULL_HANDLE};
    VmaAllocation vma_allocation_{VK_NULL_HANDLE};
    VmaAllocator vma_allocator_{VK_NULL_HANDLE};
    VkDevice vk_device_{VK_NULL_HANDLE};

    void* mapped_ptr_ = nullptr;
};

} // namespace Pathfinder
