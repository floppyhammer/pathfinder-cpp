#ifndef PATHFINDER_GPU_BUFFER_VK_H
#define PATHFINDER_GPU_BUFFER_VK_H

#include <cstdint>
#include <memory>

#include "../../common/global_macros.h"
#include "../buffer.h"
#include "base.h"

namespace Pathfinder {

class BufferVk : public Buffer {
    friend class DeviceVk;

public:
    ~BufferVk() override;

    void upload_via_mapping(size_t data_size, size_t offset, const void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    void set_label(const std::string& _label) override;

    VkBuffer get_vk_buffer();

    VkDeviceMemory get_vk_device_memory();

private:
    BufferVk(VkDevice _vk_device, const BufferDescriptor& _desc);

private:
    VkBuffer vk_buffer{};
    VkDeviceMemory vk_device_memory{};

    VkDevice vk_device{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_BUFFER_VK_H
