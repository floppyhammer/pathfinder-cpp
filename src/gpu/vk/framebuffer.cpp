#include "framebuffer.h"

#include <array>
#include <cassert>
#include <utility>

#include "debug_marker.h"

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {

FramebufferVk::FramebufferVk(VkDevice _vk_device,
                             VkRenderPass _vk_render_pass,
                             const std::shared_ptr<Texture> &_texture)
    : Framebuffer(_texture) {
    vk_device = _vk_device;

    auto texture_vk = static_cast<TextureVk *>(texture.get());

    std::array<VkImageView, 1> attachments = {texture_vk->get_image_view()};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _vk_render_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

FramebufferVk::FramebufferVk(VkDevice _vk_device, VkRenderPass _vk_render_pass, Vec2I _size, VkImageView vk_image_view)
    : Framebuffer(_size) {
    vk_device = _vk_device;
    label = "Swapchain framebuffer";

    std::array<VkImageView, 1> attachments = {vk_image_view};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _vk_render_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
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

unsigned long long FramebufferVk::get_unique_id() {
    return reinterpret_cast<unsigned long long>(vk_framebuffer);
}

void FramebufferVk::set_label(const std::string &_label) {
    Framebuffer::set_label(_label);

    DebugMarker::get_singleton()->set_object_name(vk_device,
                                                  (uint64_t)vk_framebuffer,
                                                  VK_OBJECT_TYPE_FRAMEBUFFER,
                                                  _label);
}

} // namespace Pathfinder

#endif
