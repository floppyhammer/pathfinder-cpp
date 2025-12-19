#include "swap_chain.h"

#include <cstdint>
#include <memory>

#include "command_encoder.h"
#include "debug_marker.h"
#include "window.h"
#include "window_builder.h"

namespace Pathfinder {

SwapChainVk::SwapChainVk(Vec2I size, WindowVk *window, DeviceVk *device) : SwapChain(size) {
    window_ = window;
    device_ = device;

    // Swap chain related resources.
    init_swapchain();

    create_sync_objects();
}

std::shared_ptr<RenderPass> SwapChainVk::get_render_pass() {
    return render_pass_;
}

std::shared_ptr<Texture> SwapChainVk::get_surface_texture() {
    TextureDescriptor desc{};
    desc.size = size_;
    desc.format = vk_to_texture_format(swapchain_image_format_);

    auto texture = TextureVk::from_wrapping(desc,
                                            swapchain_images_[image_index_],
                                            VK_NULL_HANDLE,
                                            swapchain_image_views_[image_index_],
                                            TextureLayout::ColorAttachment);

    return texture;
}

TextureFormat SwapChainVk::get_surface_format() const {
    return vk_to_texture_format(swapchain_image_format_);
}

bool SwapChainVk::acquire_image() {
    if (window_->is_minimized()) {
        return false;
    }

    auto vk_device = device_->get_device();

    // Wait for the CPU-side "Frame Slot" to be free.
    vkWaitForFences(vk_device, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

    // Acquire image using a semaphore dedicated to this frame slot.
    VkResult result = vkAcquireNextImageKHR(vk_device,
                                            vk_swapchain_,
                                            UINT64_MAX,
                                            image_available_semaphores_[current_frame_],
                                            VK_NULL_HANDLE,
                                            &image_index_);

    // Recreate swap chains if necessary.
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    //  Check if the image we just got is still being used by a previous frame.
    if (images_in_flight_[image_index_] != VK_NULL_HANDLE) {
        vkWaitForFences(vk_device, 1, &images_in_flight_[image_index_], VK_TRUE, UINT64_MAX);
    }

    // Mark this image as now being handled by this frame's fence.
    images_in_flight_[image_index_] = in_flight_fences_[current_frame_];

    return true;
}

void SwapChainVk::init_swapchain() {
    // Create a swap chain and corresponding swap chain images.
    create_swapchain(device_->get_physical_device());

    // Create views for swap chain images.
    create_image_views();

    create_render_pass();
}

void SwapChainVk::recreate_swapchain() {
    cleanup_swapchain();

    init_swapchain();

    images_in_flight_.resize(swapchain_images_.size(), VK_NULL_HANDLE);

    Logger::info("Swapchain recreated");
}

void SwapChainVk::cleanup_swapchain() {
    auto vk_device = device_->get_device();

    // Wait on the host for the completion of outstanding queue operations for all queues on a given logical device.
    vkDeviceWaitIdle(vk_device);

    // We don't actually have to do this.
    framebuffers_.clear();

    render_pass_.reset();

    for (auto image_view : swapchain_image_views_) {
        vkDestroyImageView(vk_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(vk_device, vk_swapchain_, nullptr);
}

void SwapChainVk::destroy() {
    auto vk_device = device_->get_device();

    encoder_of_last_frame_.reset();

    // Clean up swap chain related resources.
    cleanup_swapchain();

    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        vkDestroySemaphore(vk_device, render_finished_semaphores_[i], nullptr);
        ;
    }

    // Clean up sync objects.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vk_device, image_available_semaphores_[i], nullptr);
        vkDestroyFence(vk_device, in_flight_fences_[i], nullptr);
    }
}

void SwapChainVk::create_swapchain(VkPhysicalDevice physical_device) {
    auto vk_device = device_->get_device();

    SwapchainSupportDetails swapchain_support = query_swapchain_support(physical_device, window_->surface_);

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_modes);
    VkExtent2D vk_extent = window_->choose_swap_extent(swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }
    image_count_ = image_count;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = window_->surface_;

    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = vk_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices qf_indices = find_queue_families(physical_device, window_->surface_);
    uint32_t queue_family_indices[] = {*qf_indices.graphics_family, *qf_indices.present_family};

    if (*qf_indices.graphics_family != *qf_indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    // Create a swap chain.
    VK_CHECK_RESULT(vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &vk_swapchain_))

    // Get the number of presentable images for swap chain.
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain_, &image_count, nullptr);
    swapchain_images_.resize(image_count);

    // Obtain the array of presentable images associated with a swap chain.
    vkGetSwapchainImagesKHR(vk_device, vk_swapchain_, &image_count, swapchain_images_.data());

    swapchain_image_format_ = surface_format.format;
    size_ = {int32_t(vk_extent.width), int32_t(vk_extent.height)};
}

void SwapChainVk::create_image_views() {
    swapchain_image_views_.resize(image_count_);

    for (uint32_t i = 0; i < image_count_; i++) {
        swapchain_image_views_[i] =
            device_->create_vk_image_view(swapchain_images_[i], swapchain_image_format_, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void SwapChainVk::create_render_pass() {
    render_pass_ =
        device_->create_swap_chain_render_pass(vk_to_texture_format(swapchain_image_format_), AttachmentLoadOp::Clear);
}

void SwapChainVk::create_sync_objects() {
    auto vk_device = device_->get_device();

    image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores_.resize(image_count_);
    in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);
    images_in_flight_.resize(image_count_, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Initialize it in the signaled state

    for (size_t i = 0; i < image_count_; i++) {
        VK_CHECK_RESULT(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &render_finished_semaphores_[i]))
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK_RESULT(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &image_available_semaphores_[i]))
        VK_CHECK_RESULT(vkCreateFence(vk_device, &fence_info, nullptr, &in_flight_fences_[i]))
    }
}

void SwapChainVk::submit(const std::shared_ptr<CommandEncoder> &encoder) {
    // Cleanup last encoder.
    if (encoder_of_last_frame_) {
        encoder_of_last_frame_ = nullptr;
    }

    if (encoder->submitted_) {
        Logger::error("Attempted to submit an encoder that's already been submitted!");
        return;
    }

    encoder->submitted_ = true;

    if (!encoder->finish()) {
        return;
    }

    flush(encoder);

    encoder_of_last_frame_ = encoder;
}

void SwapChainVk::flush(const std::shared_ptr<CommandEncoder> &encoder) {
    auto vk_device = device_->get_device();
    auto vk_graphics_queue = device_->get_graphics_queue();
    auto encoder_vk = (CommandEncoderVk *)encoder.get();

    // Submit command buffer.
    // -------------------------------------
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &encoder_vk->vk_command_buffer_;

    // The semaphores to wait before commands in this command buffer can begin execution.
    VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages; // The wait will not affect stages that come before this stage.

    // The semaphores to signal after all commands in this command buffer are finished on the GPU.
    VkSemaphore signal_semaphores[] = {render_finished_semaphores_[image_index_]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkResetFences(vk_device, 1, &in_flight_fences_[current_frame_]);

    VK_CHECK_RESULT(vkQueueSubmit(vk_graphics_queue, 1, &submit_info, in_flight_fences_[current_frame_]))
    // -------------------------------------
}

void SwapChainVk::present() {
    auto vk_present_queue = device_->get_present_queue();

    // Wait until the image to present has finished rendering.
    VkSemaphore wait_semaphores[] = {render_finished_semaphores_[image_index_]};

    // Queue an image for presentation after queueing all rendering commands
    // and transitioning the image to the correct layout.
    // -------------------------------------
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Specifies the semaphores to wait for before issuing the present request.
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = wait_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk_swapchain_;
    present_info.pImageIndices = &image_index_; // Array of each swap chain's presentable images.

    VkResult result = vkQueuePresentKHR(vk_present_queue, &present_info);
    // -------------------------------------

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_->get_resize_flag()) {
        recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    // Update frame tracker.
    current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

} // namespace Pathfinder
