#include "buffer.h"

#include <cstring>
#include <stdexcept>

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

BufferVk::BufferVk(VmaAllocator vma_allocator, VkDevice vk_device, const BufferDescriptor& desc)
    : Buffer(desc), vma_allocator_(vma_allocator), vk_device_(vk_device) {}

BufferVk::~BufferVk() {
    if (mapped_ptr_) {
        unmap();
    }
    if (vma_allocator_ && vk_buffer_ && vma_allocation_) {
        vmaDestroyBuffer(vma_allocator_, vk_buffer_, vma_allocation_);
        vk_buffer_ = VK_NULL_HANDLE;
        vma_allocation_ = VK_NULL_HANDLE;
    }
}

VkBuffer BufferVk::get_vk_buffer() {
    return vk_buffer_;
}

VkDeviceMemory BufferVk::get_vk_device_memory() {
    if (!vma_allocator_ || !vma_allocation_) {
        return VK_NULL_HANDLE;
    }
    VmaAllocationInfo info{};
    vmaGetAllocationInfo(vma_allocator_, vma_allocation_, &info);
    return info.deviceMemory;
}

VmaAllocation BufferVk::get_vma_allocation() const {
    return vma_allocation_;
}

void BufferVk::upload_via_mapping(size_t data_size, size_t offset, const void* data) {
    void* mapped_data = static_cast<uint8_t*>(map()) + offset;
    memcpy(mapped_data, data, data_size);
}

void BufferVk::download_via_mapping(size_t data_size, size_t offset, void* data) {
    const void* mapped_data = static_cast<uint8_t*>(map()) + offset;
    memcpy(data, mapped_data, data_size);
}

void* BufferVk::map() {
    if (desc_.property != MemoryProperty::HostVisibleAndCoherent) {
        abort();
    }

    if (mapped_ptr_) {
        return mapped_ptr_;
    }

    if (vmaMapMemory(vma_allocator_, vma_allocation_, &mapped_ptr_) != VK_SUCCESS) {
        Logger::error("Failed to map buffer memory using VMA!");
        return nullptr;
    }

    return mapped_ptr_;
}

void BufferVk::unmap() {
    if (mapped_ptr_) {
        vmaUnmapMemory(vma_allocator_, vma_allocation_);
        mapped_ptr_ = nullptr;
    }
}

} // namespace Pathfinder
