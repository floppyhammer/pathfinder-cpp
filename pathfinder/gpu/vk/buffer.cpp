#include "buffer.h"

#include <cstring>
#include <stdexcept>

#include "debug_marker.h"
#include "device.h"

namespace Pathfinder {

BufferVk::BufferVk(VkDevice vk_device, const BufferDescriptor& desc) : Buffer(desc), vk_device_(vk_device) {}

BufferVk::~BufferVk() {
    vkDestroyBuffer(vk_device_, vk_buffer_, nullptr);
    vkFreeMemory(vk_device_, vk_device_memory_, nullptr);

    if (vk_staging_buffer_) {
        vkDestroyBuffer(vk_device_, vk_staging_buffer_, nullptr);
        vkFreeMemory(vk_device_, vk_staging_device_memory_, nullptr);
    }
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

void BufferVk::create_staging_buffer(DeviceVk* device_vk) {
    if (vk_staging_buffer_ != VK_NULL_HANDLE) {
        return;
    }

    device_vk->create_vk_buffer(get_size(),
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                vk_staging_buffer_,
                                vk_staging_device_memory_);
}

} // namespace Pathfinder
