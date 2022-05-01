#include "framebuffer.h"

#include <cassert>
#include <array>
#include <utility>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    FramebufferVk::FramebufferVk(VkDevice device,
                                 VkRenderPass render_pass,
                                 std::shared_ptr<Texture> p_texture,
                                 uint32_t p_width,
                                 uint32_t p_height,
                                 TextureFormat p_format) : Framebuffer(p_width, p_height) {
        vk_device = device;

        // Set color texture.
        texture = std::move(p_texture);

        auto texture_vk = static_cast<TextureVk *>(texture.get());

        std::array<VkImageView, 1> attachments = {texture_vk->get_image_view()};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }

        // Fill a descriptor for later use in a descriptor set.
        descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor.imageView = texture_vk->get_image_view();
        descriptor.sampler = texture_vk->get_sampler();
    }

    FramebufferVk::FramebufferVk(VkDevice device,
                                 VkRenderPass render_pass,
                                 uint32_t p_width,
                                 uint32_t p_height,
                                 VkImageView image_view)
            : Framebuffer(p_width, p_height) {
        vk_device = device;

        std::array<VkImageView, 1> attachments = {image_view};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    FramebufferVk::~FramebufferVk() {
        vkDestroyFramebuffer(vk_device, vk_framebuffer, nullptr);
    }

    VkFramebuffer FramebufferVk::get_vk_framebuffer() const {
        return vk_framebuffer;
    }

    uint32_t FramebufferVk::get_unique_id() {
        return reinterpret_cast<uint32_t>(&vk_framebuffer);;
    }
}

#endif
