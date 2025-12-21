#ifndef PATHFINDER_GPU_DRIVER_VK_H
#define PATHFINDER_GPU_DRIVER_VK_H

#include "../base.h"
#include "../device.h"
#include "render_pass.h"

namespace Pathfinder {
class Fence;

class DeviceVk : public Device {
    friend class WindowVk;
    friend class SwapChainVk;
    friend class WindowBuilderVk;

public:
    DeviceVk(VkDevice vk_device,
             VkPhysicalDevice vk_physical_device,
             VkQueue vk_graphics_queue,
             VkQueue vk_present_queue,
             VkCommandPool vk_command_pool);

    std::shared_ptr<RenderPass> create_render_pass(TextureFormat format,
                                                   AttachmentLoadOp load_op,
                                                   const std::string &label) override;

    std::shared_ptr<RenderPass> create_swap_chain_render_pass(TextureFormat format, AttachmentLoadOp load_op) override;

    std::shared_ptr<Framebuffer> create_framebuffer(const std::shared_ptr<RenderPass> &render_pass,
                                                    const std::shared_ptr<Texture> &texture,
                                                    const std::string &label) override;

    std::shared_ptr<Buffer> create_buffer(const BufferDescriptor &desc, const std::string &label) override;

    std::shared_ptr<Texture> create_texture(const TextureDescriptor &desc, const std::string &label) override;

    std::shared_ptr<Sampler> create_sampler(SamplerDescriptor descriptor) override;

    std::shared_ptr<CommandEncoder> create_command_encoder(const std::string &label) override;

    std::shared_ptr<DescriptorSet> create_descriptor_set() override;

    std::shared_ptr<ShaderModule> create_shader_module(const std::vector<char> &source_code,
                                                       ShaderStage shader_stage,
                                                       const std::string &label) override;

    std::shared_ptr<RenderPipeline> create_render_pipeline(
        const std::shared_ptr<ShaderModule> &vert_shader_module,
        const std::shared_ptr<ShaderModule> &frag_shader_module,
        const std::vector<VertexInputAttributeDescription> &attribute_descriptions,
        BlendState blend_state,
        const std::shared_ptr<DescriptorSet> &descriptor_set,
        TextureFormat target_format,
        const std::string &label) override;

    std::shared_ptr<ComputePipeline> create_compute_pipeline(const std::shared_ptr<ShaderModule> &comp_shader_module,
                                                             const std::shared_ptr<DescriptorSet> &descriptor_set,
                                                             const std::string &label) override;

    std::shared_ptr<Fence> create_fence(const std::string &label) override;

    VkDevice get_device() const;

    VkPhysicalDevice get_physical_device() const;

    VkQueue get_graphics_queue() const;

    VkQueue get_present_queue() const;

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
    VkPhysicalDevice vk_physical_device_{};

    VkDevice vk_device_{};

    VkQueue vk_graphics_queue_{};

    VkQueue vk_present_queue_{};

    VkCommandPool vk_command_pool_{};

    VkShaderModule create_shader_module(const std::vector<char> &code);

    void create_vk_image(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage &image,
                         VkDeviceMemory &image_memory,
                         size_t &memory_size) const;

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
