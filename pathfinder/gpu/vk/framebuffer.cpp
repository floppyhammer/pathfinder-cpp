#include "framebuffer.h"

#include <array>
#include <cassert>

#include "debug_marker.h"

namespace Pathfinder {

FramebufferVk::FramebufferVk(VkDevice vk_device, VkRenderPass vk_render_pass, const std::shared_ptr<Texture> &texture)
    : Framebuffer(texture) {
    vk_device_ = vk_device;

    auto texture_vk = static_cast<TextureVk *>(texture.get());

    std::array<VkImageView, 1> attachments = {texture_vk->get_image_view()};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vk_render_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size_.x;
    framebufferInfo.height = size_.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_framebuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

FramebufferVk::FramebufferVk(VkDevice vk_device, VkRenderPass vk_render_pass, Vec2I size, VkImageView vk_image_view)
    : Framebuffer(size) {
    vk_device_ = vk_device;
    label_ = "Swapchain framebuffer";

    std::array<VkImageView, 1> attachments = {vk_image_view};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vk_render_pass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(vk_device, &framebufferInfo, nullptr, &vk_framebuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

FramebufferVk::~FramebufferVk() {
    vkDestroyFramebuffer(vk_device_, vk_framebuffer_, nullptr);
}

VkFramebuffer FramebufferVk::get_vk_framebuffer() const {
    return vk_framebuffer_;
}

unsigned long long FramebufferVk::get_unique_id() {
    return reinterpret_cast<unsigned long long>(vk_framebuffer_);
}

void FramebufferVk::set_label(const std::string &label) {
    Framebuffer::set_label(label);

    DebugMarker::get_singleton()->set_object_name(vk_device_,
                                                  (uint64_t)vk_framebuffer_,
                                                  VK_OBJECT_TYPE_FRAMEBUFFER,
                                                  label);
}

} // namespace Pathfinder
