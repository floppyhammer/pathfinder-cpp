#include "swap_chain.h"

#include "platform.h"
#include "command_buffer.h"

#include <cstdint>
#include <memory>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    SwapChainVk::SwapChainVk(uint32_t p_width,
                             uint32_t p_height,
                             PlatformVk *p_platform,
                             DriverVk *p_driver) : SwapChain(p_width, p_height) {
        platform = p_platform;
        driver = p_driver;

        // Swap chain related resources.
        init_swapchain();

        create_sync_objects();
    }

    std::shared_ptr<RenderPass> SwapChainVk::get_render_pass() {
        return render_pass;
    }

    std::shared_ptr<Framebuffer> SwapChainVk::get_framebuffer() {
        return framebuffers[current_image];
    }

    std::shared_ptr<CommandBuffer> SwapChainVk::get_command_buffer() {
        auto command_buffer_vk = std::make_shared<CommandBufferVk>(command_buffers[current_image], driver->device);
        return command_buffer_vk;
    }

    bool SwapChainVk::acquire_image() {
        return acquire_swapchain_image(current_image);
    }

    void SwapChainVk::init_swapchain() {
        // Create a swap chain and corresponding swap chain images.
        create_swapchain();

        // Create views for swap chain images.
        create_image_views();

        create_render_pass();

        create_framebuffers();

        // Create a command buffer for each swap chain image.
        create_command_buffers();
    }

    void SwapChainVk::recreate_swapchain() {
        // Handling window minimization.
//        int width = 0, height = 0;
//        glfwGetFramebufferSize(window, &width, &height);
//        while (width == 0 || height == 0) {
//            glfwGetFramebufferSize(window, &width, &height);
//            glfwWaitEvents();
//        }

        cleanup_swapchain();

        init_swapchain();

        images_in_flight.resize(swapchain_images.size(), VK_NULL_HANDLE);
    }

    void SwapChainVk::cleanup_swapchain() {
        auto device = driver->get_device();
        auto command_pool = driver->get_command_pool();

        // Wait on the host for the completion of outstanding queue operations for all queues on a given logical device.
        vkDeviceWaitIdle(device);

        // Only command buffers are freed but not the pool.
        vkFreeCommandBuffers(device,
                             command_pool,
                             static_cast<uint32_t>(command_buffers.size()),
                             command_buffers.data());

        // We don't actually have to do this.
        framebuffers.clear();

        render_pass.reset();

        for (auto imageView: swapchain_image_views) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void SwapChainVk::cleanup() {
        auto device = driver->get_device();

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
        auto device = driver->get_device();

        SwapchainSupportDetails swapchain_support = platform->query_swapchain_support(platform->physical_device);

        VkSurfaceFormatKHR surface_format = platform->choose_swap_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = platform->choose_swap_present_mode(swapchain_support.present_modes);
        VkExtent2D extent = platform->choose_swap_extent(swapchain_support.capabilities);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 &&
            image_count > swapchain_support.capabilities.maxImageCount) {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = platform->surface;

        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices qf_indices = platform->find_queue_families(platform->physical_device);
        uint32_t queueFamilyIndices[] = {qf_indices.graphics_family.value(), qf_indices.present_family.value()};

        if (qf_indices.graphics_family != qf_indices.present_family) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queueFamilyIndices;
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
        swapchain_extent = extent;
    }

    void SwapChainVk::create_image_views() {
        swapchain_image_views.resize(swapchain_images.size());

        for (uint32_t i = 0; i < swapchain_images.size(); i++) {
            swapchain_image_views[i] = driver->create_vk_image_view(swapchain_images[i],
                                                                    swapchain_image_format,
                                                                    VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void SwapChainVk::create_render_pass() {
        render_pass = driver->create_render_pass(vk_to_texture_format(swapchain_image_format),
                                                 AttachmentLoadOp::CLEAR,
                                                 ImageLayout::PRESENT_SRC);
    }

    void SwapChainVk::create_framebuffers() {
        auto device = driver->get_device();

        framebuffers.clear();

        for (size_t i = 0; i < swapchain_images.size(); i++) {
            auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

            // No texture for swap chain framebuffer.
            auto framebuffer_vk = std::make_shared<FramebufferVk>(
                    device,
                    render_pass_vk->get_vk_render_pass(),
                    extent.x,
                    extent.y,
                    swapchain_image_views[i]);

            framebuffers.push_back(framebuffer_vk);
        }
    }

    void SwapChainVk::create_sync_objects() {
        auto device = driver->get_device();

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

    bool SwapChainVk::acquire_swapchain_image(uint32_t &image_index) {
        auto device = driver->get_device();

        // Wait for the frame to be finished.
        vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

        // Retrieve the index of the next available presentable image.
        VkResult result = vkAcquireNextImageKHR(device,
                                                swapchain,
                                                UINT64_MAX,
                                                image_available_semaphores[current_frame],
                                                VK_NULL_HANDLE,
                                                &image_index);

        current_image = image_index;

        // Recreate swap chains if necessary.
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return false;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        return true;
    }

    void SwapChainVk::create_command_buffers() {
        auto device = driver->get_device();
        auto command_pool = driver->get_command_pool();

        command_buffers.resize(framebuffers.size());

        // Allocate command buffers.
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = (uint32_t) command_buffers.size();

        if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void SwapChainVk::flush() {
        auto device = driver->get_device();
        auto graphics_queue = driver->get_graphics_queue();
        auto present_queue = driver->get_present_queue();

        auto imageIndex = current_image;

        if (images_in_flight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &images_in_flight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        images_in_flight[imageIndex] = in_flight_fences[current_frame];

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
        submit_info.pCommandBuffers = &command_buffers[imageIndex];

        // The semaphores to signal after all commands in the buffer are finished.
        VkSemaphore signal_semaphores[] = {render_finished_semaphores[current_frame]};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        vkResetFences(device, 1, &in_flight_fences[current_frame]);

        if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }
        // -------------------------------------

        // Queue an image for presentation after queueing all rendering commands and transitioning the image to the correct layout.
        // -------------------------------------
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // Specifies the semaphores to wait for before issuing the present request.
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapChains[] = {swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapChains;

        // Array of each swap chain’s presentable images.
        present_info.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(present_queue, &present_info);
        // -------------------------------------

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || platform->framebuffer_resized) {
            platform->framebuffer_resized = false;
            recreate_swapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

#endif
