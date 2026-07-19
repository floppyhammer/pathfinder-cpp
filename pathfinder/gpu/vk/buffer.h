#pragma once

#include "../buffer.h"
#include "base.h"

namespace Pathfinder {

class DeviceVk;

class BufferVk : public Buffer {
    friend class DeviceVk;
    friend class CommandEncoderVk;

public:
    ~BufferVk() override;

    void upload_via_mapping(size_t data_size, size_t offset, const void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    void set_label(const std::string& label) override;

    VkBuffer get_vk_buffer();

    VkDeviceMemory get_vk_device_memory();

    /// Map the whole buffer once, offset is zero.
    void* map();

    void unmap();

private:
    BufferVk(VkDevice vk_device, const BufferDescriptor& desc);

    VkBuffer vk_buffer_{};
    VkDeviceMemory vk_device_memory_{};

    VkDevice vk_device_{};

    void* mapped_ptr_ = nullptr;
};

} // namespace Pathfinder
