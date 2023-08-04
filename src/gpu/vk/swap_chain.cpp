#include "swap_chain.h"

#include <cstdint>
#include <memory>

#include "command_encoder.h"
#include "debug_marker.h"
#include "window.h"

#ifdef PATHFINDER_USE_VULKAN
    #if (defined(WIN32) || defined(__linux__) || defined(__APPLE__)) && !defined(ANDROID)

namespace Pathfinder {

SwapChainVk::SwapChainVk(Vec2I _size, WindowVk *_window, DeviceVk *_device) : SwapChain(_size) {
    window = _window;
    device_vk = _device;

    // Swap chain related resources.
    init_swapchain();

    create_sync_objects();
}

std::shared_ptr<RenderPass> SwapChainVk::get_render_pass() {
    return render_pass;
}

std::shared_ptr<Framebuffer> SwapChainVk::get_framebuffer() {
    return framebuffers[image_index];
}

bool SwapChainVk::acquire_image() {
    if (window->is_minimized()) {
        return false;
    }

    auto device = device_vk->get_device();

    // Wait for the frame to be finished.
    vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    // Retrieve the index of the next available presentable image.
    VkResult result = vkAcquireNextImageKHR(device,
                                            swapchain,
                                            UINT64_MAX,
                                            image_available_semaphores[current_frame],
                                            VK_NULL_HANDLE,
                                            &image_index);

    // Recreate swap chains if necessary.
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    return true;
}

void SwapChainVk::init_swapchain() {
    // Create a swap chain and corresponding swap chain images.
    create_swapchain();

    // Create views for swap chain images.
    create_image_views();

    create_render_pass();

    create_framebuffers();
}

void SwapChainVk::recreate_swapchain() {
    cleanup_swapchain();

    init_swapchain();

    images_in_flight.resize(swapchain_images.size(), VK_NULL_HANDLE);

    Logger::info("Swapchain recreated");
}

void SwapChainVk::cleanup_swapchain() {
    auto device = device_vk->get_device();

    // Wait on the host for the completion of outstanding queue operations for all queues on a given logical device.
    vkDeviceWaitIdle(device);

    // We don't actually have to do this.
    framebuffers.clear();

    render_pass.reset();

    for (auto imageView : swapchain_image_views) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void SwapChainVk::cleanup() {
    auto device = device_vk->get_device();

    // Clean up swap chain related resources.
    cleanup_swapchain();

    // Clean up sync objects.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
        vkDestroyFence(device, in_flight_fences[i], nullptr);
    }
}

void SwapChainVk::create_swapchain() {
    auto device = device_vk->get_device();

    SwapchainSupportDetails swapchain_support = window->query_swapchain_support(window->physical_device);

    VkSurfaceFormatKHR surface_format = window->choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = window->choose_swap_present_mode(swapchain_support.present_modes);
    VkExtent2D vk_extent = window->choose_swap_extent(swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = window->surface;

    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = vk_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices qf_indices = window->find_queue_families(window->physical_device);
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
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    // Get the number of presentable images for swap chain.
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);

    // Obtain the array of presentable images associated with a swap chain.
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());

    swapchain_image_format = surface_format.format;
    size = {int32_t(vk_extent.width), int32_t(vk_extent.height)};
}

void SwapChainVk::create_image_views() {
    swapchain_image_views.resize(swapchain_images.size());

    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        swapchain_image_views[i] =
            device_vk->create_vk_image_view(swapchain_images[i], swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void SwapChainVk::create_render_pass() {
    render_pass =
        device_vk->create_swap_chain_render_pass(vk_to_texture_format(swapchain_image_format), AttachmentLoadOp::Clear);
}

void SwapChainVk::create_framebuffers() {
    auto device = device_vk->get_device();

    framebuffers.clear();

    for (size_t i = 0; i < swapchain_images.size(); i++) {
        auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

        // No texture for swap chain framebuffer.
        auto framebuffer_vk = std::shared_ptr<FramebufferVk>(
            new FramebufferVk(device, render_pass_vk->get_vk_render_pass(), size, swapchain_image_views[i]));

        framebuffers.push_back(framebuffer_vk);
    }
}

void SwapChainVk::create_sync_objects() {
    auto device = device_vk->get_device();

    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    images_in_flight.resize(swapchain_images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Initialize it in the signaled state

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void SwapChainVk::flush(const std::shared_ptr<CommandEncoder> &encoder) {
    auto device = device_vk->get_device();
    auto graphics_queue = device_vk->get_graphics_queue();
    auto present_queue = window->get_present_queue();
    auto encoder_vk = (CommandEncoderVk *)encoder.get();

    if (images_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    images_in_flight[image_index] = in_flight_fences[current_frame];

    // Submit command buffer.
    // -------------------------------------
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &encoder_vk->vk_command_buffer;

    // The semaphores to signal after all commands in the buffer are finished.
    VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkResetFences(device, 1, &in_flight_fences[current_frame]);

    if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit queue!");
    }
    // -------------------------------------
}

void SwapChainVk::SwapChainVk::present() {
    auto present_queue = window->get_present_queue();

    // Wait until the image to present has finished rendering.
    VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};

    // Queue an image for presentation after queueing all rendering commands
    // and transitioning the image to the correct layout.
    // -------------------------------------
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Specifies the semaphores to wait for before issuing the present request.
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;

    // Array of each swap chain's presentable images.
    present_info.pImageIndices = &image_index;

    VkResult result = vkQueuePresentKHR(present_queue, &present_info);
    // -------------------------------------

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->get_resize_flag()) {
        recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    // Update frame tracker.
    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

} // namespace Pathfinder

    #endif
#endif
