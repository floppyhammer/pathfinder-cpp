#ifndef PATHFINDER_GPU_BUFFER_VK_H
#define PATHFINDER_GPU_BUFFER_VK_H

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

private:
    BufferVk(VkDevice vk_device, const BufferDescriptor& desc);

    void create_staging_buffer(DeviceVk* device_vk);

    VkBuffer vk_buffer_{};
    VkDeviceMemory vk_device_memory_{};

    VkBuffer vk_staging_buffer_{};
    VkDeviceMemory vk_staging_device_memory_{};

    VkDevice vk_device_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_VK_H
