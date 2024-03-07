#include "render_pass.h"

#include <cassert>

#include "debug_marker.h"

namespace Pathfinder {

RenderPassVk::RenderPassVk(VkDevice vk_device,
                           TextureFormat texture_format,
                           AttachmentLoadOp load_op,
                           bool is_swap_chain_pass,
                           const std::string &label) {
    vk_device_ = vk_device;
    label_ = label;
    load_op_ = load_op;

    // Color attachment.
    // ----------------------------------------
    VkAttachmentDescription colorAttachment{};
    // Specifying the format of the image view that will be used for the attachment.
    colorAttachment.format = to_vk_texture_format(texture_format);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Specifying the number of samples of the image.
    // Specifying how the contents of color and depth components of the attachment are treated at the beginning of the
    // sub-pass where it is first used.
    colorAttachment.loadOp = to_vk_attachment_load_op(load_op);
    // Specifying how the contents of color and depth components of the attachment are treated at the end of the
    // sub-pass where it is last used.
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // The layout the attachment image subresource will be in when a render pass instance begins.
    colorAttachment.initialLayout =
        load_op == AttachmentLoadOp::Clear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // The layout the attachment image subresource will be transitioned to when a render pass instance ends.
    colorAttachment.finalLayout =
        is_swap_chain_pass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    // Specifying the layout the attachment uses during the sub-pass.
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // ----------------------------------------

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK_RESULT(vkCreateRenderPass(vk_device, &renderPassInfo, nullptr, &vk_render_pass_))

    DebugMarker::get_singleton()->set_object_name(vk_device,
                                                  (uint64_t)vk_render_pass_,
                                                  VK_OBJECT_TYPE_RENDER_PASS,
                                                  label);
}

RenderPassVk::~RenderPassVk() {
    vkDestroyRenderPass(vk_device_, vk_render_pass_, nullptr);
}

VkRenderPass RenderPassVk::get_vk_render_pass() {
    return vk_render_pass_;
}

} // namespace Pathfinder
