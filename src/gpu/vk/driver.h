#ifndef PATHFINDER_GPU_DRIVER_VK_H
#define PATHFINDER_GPU_DRIVER_VK_H

#include "render_pass.h"
#include "../driver.h"
#include "../data.h"
#include "../../common/io.h"
#include "../../common/global_macros.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class DriverVk : public Driver {
        friend class PlatformVk;
        friend class SwapChainVk;
    public:
        DriverVk(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkQueue presentQueue,
                 VkCommandPool commandPool);

        std::shared_ptr<RenderPass> create_render_pass(TextureFormat format, AttachmentLoadOp load_op, ImageLayout final_layout) override;

        std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                        const std::shared_ptr<Texture> &texture) override;

        std::shared_ptr<Buffer> create_buffer(BufferType type, size_t size, MemoryProperty property) override;

        std::shared_ptr<Texture> create_texture(uint32_t width,
                                                uint32_t height,
                                                TextureFormat format) override;

        std::shared_ptr<CommandBuffer> create_command_buffer(bool one_time) override;

        std::shared_ptr<DescriptorSet> create_descriptor_set() override;

        std::shared_ptr<RenderPipeline> create_render_pipeline(const std::vector<char> &vert_source,
                                                               const std::vector<char> &frag_source,
                                                               const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
                                                               ColorBlendState blend_state,
                                                               const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                               const std::shared_ptr<RenderPass> &render_pass) override;

        std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                                 const std::shared_ptr<DescriptorSet> &descriptor_set) override;

    public:
        VkDevice get_device() const;

        VkQueue get_graphics_queue() const;

        VkQueue get_present_queue() const;

        VkCommandPool get_command_pool() const;

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        void createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer &buffer,
                            VkDeviceMemory &bufferMemory);

        void copyDataToMemory(const void *src, VkDeviceMemory bufferMemory, size_t dataSize) const;

        void transitionImageLayout(VkCommandBuffer commandBuffer,
                                   VkImage image,
                                   VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout) const;

        void copyVkBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    private:
        /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        VkDevice device{};

        VkQueue graphicsQueue, presentQueue;

        VkCommandPool commandPool;

        VkShaderModule createShaderModule(const std::vector<char> &code);

        void createVkImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                           VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                           VkImage &image, VkDeviceMemory &imageMemory) const;

        VkImageView createVkImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

        void createVkTextureSampler(VkSampler &textureSampler) const;

        void createVkRenderPass(VkFormat format, VkRenderPass &renderPass);

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags features) const;

        VkFormat findDepthFormat() const;

        void copyBufferToImage(VkCommandBuffer commandBuffer,
                               VkBuffer buffer,
                               VkImage image,
                               uint32_t width,
                               uint32_t height) const;
    };
}

#endif

#endif //PATHFINDER_GPU_DRIVER_VK_H
