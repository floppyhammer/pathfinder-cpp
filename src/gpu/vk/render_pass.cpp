#include "render_pass.h"

namespace Pathfinder {
    Pathfinder::RenderPassVk::RenderPassVk(VkDevice device, TextureFormat texture_format) {
        vk_device = device;

        // Color attachment.
        // ----------------------------------------
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = to_vk_texture_format(texture_format); // Specifying the format of the image view that will be used for the attachment.
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Specifying the number of samples of the image.
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Specifying how the contents of color and depth components of the attachment are treated at the beginning of the subpass where it is first used.
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Specifying how the contents of color and depth components of the attachment are treated at the end of the subpass where it is last used.
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // The layout the attachment image subresource will be in when a render pass instance begins.
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // The layout the attachment image subresource will be transitioned to when a render pass instance ends.

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Specifying the layout the attachment uses during the subpass.
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

        if (vkCreateRenderPass(vk_device, &renderPassInfo, nullptr, &vk_render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    RenderPassVk::~RenderPassVk() {
        vkDestroyRenderPass(vk_device, vk_render_pass, nullptr);
    }

    VkRenderPass RenderPassVk::get_vk_render_pass() {
        return vk_render_pass;
    }
}
