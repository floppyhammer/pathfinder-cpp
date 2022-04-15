#ifndef PATHFINDER_DEVICE_VK_H
#define PATHFINDER_DEVICE_VK_H

#include "../vertex_input.h"
#include "../buffer.h"
#include "../texture.h"
#include "../command_buffer.h"
#include "../../common/io.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DeviceVk {
    public:
        static DeviceVk &getSingleton() {
            static DeviceVk singleton;
            return singleton;
        }

        void init(VkDevice p_device);

    public:
        void create_pipeline(std::vector<char> vertShaderCode,
                             std::vector<char> fragShaderCode,
                             std::vector<VertexInputAttributeDescription> pDescriptions);

        static std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) override;

        static std::shared_ptr<Texture> create_texture(uint32_t p_width, uint32_t p_height, TextureFormat p_format, DataType p_type);

        static std::shared_ptr<CommandBuffer> create_command_buffer();

        VkDevice get_device() const;

    private:
        VkShaderModule createShaderModule(const std::vector<char> &code);

        VkDevice device{}
    };
}

#endif

#endif //PATHFINDER_DEVICE_VK_H
