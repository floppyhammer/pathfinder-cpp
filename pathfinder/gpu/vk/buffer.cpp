#include "buffer.h"

#include <cstring>
#include <stdexcept>

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

BufferVk::BufferVk(VkDevice vk_device, const BufferDescriptor& desc) : Buffer(desc), vk_device_(vk_device) {}

BufferVk::~BufferVk() {
    if (mapped_ptr_) {
        unmap();
    }
    vkDestroyBuffer(vk_device_, vk_buffer_, nullptr);
    vkFreeMemory(vk_device_, vk_device_memory_, nullptr);
}

VkBuffer BufferVk::get_vk_buffer() {
    return vk_buffer_;
}

VkDeviceMemory BufferVk::get_vk_device_memory() {
    return vk_device_memory_;
}

void BufferVk::upload_via_mapping(size_t data_size, size_t offset, const void* data) {
    if (desc_.property != MemoryProperty::HostVisibleAndCoherent) {
        abort();
    }

    void* mapped_data;
    auto res = vkMapMemory(vk_device_, vk_device_memory_, offset, data_size, 0, &mapped_data);

    if (res != VK_SUCCESS) {
        Logger::error("Failed to map memory!");
        return;
    }

    memcpy(mapped_data, data, data_size);
    vkUnmapMemory(vk_device_, vk_device_memory_);
}

void BufferVk::download_via_mapping(size_t data_size, size_t offset, void* data) {
    if (desc_.property != MemoryProperty::HostVisibleAndCoherent) {
        abort();
    }

    void* mapped_data;
    auto res = vkMapMemory(vk_device_, vk_device_memory_, offset, data_size, 0, &mapped_data);

    if (res != VK_SUCCESS) {
        Logger::error("Failed to map memory!");
        return;
    }

    memcpy(data, mapped_data, data_size);
    vkUnmapMemory(vk_device_, vk_device_memory_);
}

void BufferVk::set_label(const std::string& label) {
    assert(vk_device_ != nullptr && vk_buffer_ != nullptr);

    Buffer::set_label(label);

    DebugMarker::get_singleton()->set_object_name(vk_device_, (uint64_t)vk_buffer_, VK_OBJECT_TYPE_BUFFER, label);
}

void* BufferVk::map() {
    if (mapped_ptr_) {
        return mapped_ptr_;
    }

    if (vkMapMemory(vk_device_, vk_device_memory_, 0, get_size(), 0, &mapped_ptr_) != VK_SUCCESS) {
        Logger::error("Failed to map buffer memory!");
        return nullptr;
    }

    return mapped_ptr_;
}

void BufferVk::unmap() {
    if (mapped_ptr_) {
        vkUnmapMemory(vk_device_, vk_device_memory_);
        mapped_ptr_ = nullptr;
    }
}

} // namespace Pathfinder
