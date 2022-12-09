#ifndef PATHFINDER_GPU_DRIVER_VK_H
#define PATHFINDER_GPU_DRIVER_VK_H

#include "../../common/global_macros.h"
#include "../../common/io.h"
#include "../data.h"
#include "../driver.h"
#include "render_pass.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class DriverVk : public Driver {
    friend class PlatformVk;

    friend class SwapChainVk;

public:
    DriverVk(VkDevice device, VkPhysicalDevice physical_device, VkQueue graphics_queue, VkCommandPool command_pool);

    std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                   AttachmentLoadOp load_op,
                                                   bool is_swapchain_render_pass,
                                                   const std::string &label) override;

    std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                    const std::shared_ptr<Texture> &texture,
                                                    const std::string &_label) override;

    std::shared_ptr<Buffer> create_buffer(BufferType type,
                                          size_t size,
                                          MemoryProperty property,
                                          const std::string &_label) override;

    std::shared_ptr<Texture> create_texture(Vec2I size, TextureFormat format, const std::string &_label) override;

    std::shared_ptr<CommandBuffer> create_command_buffer(const std::string &_label) override;

    std::shared_ptr<DescriptorSet> create_descriptor_set() override;

    std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::vector<char> &vert_source,
        const std::vector<char> &frag_source,
        const std::vector<VertexInputAttributeDescription> &p_attribute_descriptions,
        BlendState blend_state,
        const std::shared_ptr<DescriptorSet> &descriptor_set,
        const std::shared_ptr<RenderPass> &render_pass,
        const std::string &_label) override;

    std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::vector<char> &comp_source,
                                                             const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                             const std::string &_label) override;

public:
    VkDevice get_device() const;

    VkQueue get_graphics_queue() const;

    VkCommandPool get_command_pool() const;

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    void create_vk_buffer(VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer &buffer,
                          VkDeviceMemory &buffer_memory);

    void copy_data_to_mappable_memory(const void *src, VkDeviceMemory buffer_memory, size_t data_size) const;

    void copy_data_from_mappable_memory(void *dst, VkDeviceMemory buffer_memory, size_t data_size) const;

    void copy_vk_buffer(VkCommandBuffer command_buffer,
                        VkBuffer src_buffer,
                        VkBuffer dst_buffer,
                        VkDeviceSize size,
                        VkDeviceSize src_offset = 0,
                        VkDeviceSize dst_offset = 0) const;

private:
    /// The graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle.
    VkPhysicalDevice physical_device{};

    VkDevice device{};

    // Note that we don't need the present queue in Driver.
    VkQueue graphics_queue{};

    VkCommandPool command_pool{};

private:
    VkShaderModule create_shader_module(const std::vector<char> &code);

    void create_vk_image(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage &image,
                         VkDeviceMemory &image_memory) const;

    VkImageView create_vk_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) const;

    VkSampler create_vk_sampler() const;

    VkFormat find_supported_format(const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features) const;

    VkFormat find_depth_format() const;

    void copy_buffer_to_image(VkCommandBuffer command_buffer,
                              VkBuffer buffer,
                              VkImage image,
                              uint32_t width,
                              uint32_t height) const;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_DRIVER_VK_H
