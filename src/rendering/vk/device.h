#ifndef PATHFINDER_HAL_DEVICE_VK_H
#define PATHFINDER_HAL_DEVICE_VK_H

#include "../device.h"
#include "../vertex_input.h"
#include "../buffer.h"
#include "../command_buffer.h"
#include "../../common/io.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DeviceVk : public Device {
    public:
        std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format,
                                                        DataType p_type) override;

        std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) override;

        std::shared_ptr<Texture> create_texture(uint32_t p_width,
                                                uint32_t p_height,
                                                TextureFormat p_format,
                                                DataType p_type) override;

        std::shared_ptr<CommandBuffer> create_command_buffer() override;

        std::shared_ptr<RenderPipeline> create_render_pipeline(const std::string &vert_source,
                                                               const std::string &frag_source,
                                                               const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                               ColorBlendState blend_state) override;

        std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::string &comp_source) override;

        VkDevice get_device() const;

    private:
        VkShaderModule createShaderModule(const std::vector<char> &code);

        VkInstance instance{};
        VkDebugUtilsMessengerEXT debugMessenger{};

        VkSurfaceKHR surface{};

        /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        VkDevice device{};
    };
}

#endif

#endif //PATHFINDER_HAL_DEVICE_VK_H
