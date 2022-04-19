#ifndef PATHFINDER_GPU_DRIVER_VK_H
#define PATHFINDER_GPU_DRIVER_VK_H

#include "../driver.h"
#include "../vertex_input.h"
#include "../../common/io.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DriverVk : public Driver {
        friend class PlatformVk;
    public:
        DriverVk(VkDevice device, VkPhysicalDevice physicalDevice);

        std::shared_ptr<SwapChain> create_swap_chain(uint32_t p_width, uint32_t p_height) override;

        std::shared_ptr<RenderPass> create_render_pass() override;

        std::shared_ptr<Framebuffer> create_framebuffer(uint32_t p_width,
                                                        uint32_t p_height,
                                                        TextureFormat p_format,
                                                        DataType p_type,
                                                        const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size) override;

        std::shared_ptr<Texture> create_texture(uint32_t width,
                                                uint32_t height,
                                                TextureFormat format,
                                                DataType type) override;

        std::shared_ptr<CommandBuffer> create_command_buffer() override;

        std::shared_ptr<RenderPipeline> create_render_pipeline(const std::vector<char> &vert_source,
                                                               const std::vector<char> &frag_source,
                                                               const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                               ColorBlendState blend_state,
                                                               const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                               const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                                 const std::shared_ptr<DescriptorSet> &descriptor_set) override;

        VkDevice get_device() const;

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    private:
        /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        VkDevice device{};

        VkShaderModule createShaderModule(const std::vector<char> &code);

        void createVkImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                           VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                           VkImage &image, VkDeviceMemory &imageMemory) const;

        VkImageView createVkImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

        void createVkTextureSampler(VkSampler &textureSampler) const;

        void createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer &buffer,
                            VkDeviceMemory &bufferMemory);

        void createVkRenderPass(VkFormat format, VkRenderPass &renderPass);

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags features) const;

        VkFormat findDepthFormat() const;
    };
}

#endif

#endif //PATHFINDER_GPU_DRIVER_VK_H
