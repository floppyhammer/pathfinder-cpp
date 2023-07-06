#ifndef PATHFINDER_GPU_BUFFER_VK_H
#define PATHFINDER_GPU_BUFFER_VK_H

#include <cstdint>
#include <memory>

#include "../../common/global_macros.h"
#include "../buffer.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class BufferVk : public Buffer {
    friend class DeviceVk;

public:
    BufferVk(VkDevice _vk_device, const BufferDescriptor& _desc);

    ~BufferVk();

    void upload_via_mapping(size_t data_size, size_t offset, void* data) override;

    void download_via_mapping(size_t data_size, size_t offset, void* data) override;

    void set_label(const std::string& _label) override;

    VkBuffer get_vk_buffer();

    VkDeviceMemory get_vk_device_memory();

private:
    VkBuffer vk_buffer{};
    VkDeviceMemory vk_device_memory{};

    VkDevice vk_device{};
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_BUFFER_VK_H
