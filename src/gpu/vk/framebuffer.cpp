#include "framebuffer.h"

#include <array>
#include <cassert>
#include <utility>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
FramebufferVk::FramebufferVk(VkDevice _device, VkRenderPass render_pass, const std::shared_ptr<Texture> &_texture)
    : Framebuffer(_texture) {
    device = _device;

    auto texture_vk = static_cast<TextureVk *>(texture.get());

    std::array<VkImageView, 1> attachments = {texture_vk->get_image_view()};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = render_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &vk_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

FramebufferVk::FramebufferVk(VkDevice _device, VkRenderPass render_pass, Vec2I _size, VkImageView image_view)
    : Framebuffer(_size) {
    device = _device;

    std::array<VkImageView, 1> attachments = {image_view};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = render_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &vk_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

FramebufferVk::~FramebufferVk() {
    vkDestroyFramebuffer(device, vk_framebuffer, nullptr);
}

VkFramebuffer FramebufferVk::get_vk_framebuffer() const {
    return vk_framebuffer;
}

unsigned long long FramebufferVk::get_unique_id() {
    return reinterpret_cast<unsigned long long>(vk_framebuffer);
}
} // namespace Pathfinder

#endif
