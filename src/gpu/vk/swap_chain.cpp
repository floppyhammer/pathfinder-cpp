#include "swap_chain.h"

#include "platform.h"
#include "command_buffer.h"

#include <cstdint>
#include <memory>
#include <array>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    SwapChainVk::SwapChainVk(uint32_t p_width,
                             uint32_t p_height,
                             PlatformVk *p_platform,
                             DriverVk *p_driver) : SwapChain(p_width, p_height) {
        platform = p_platform;
        driver = p_driver;

        // Swap chain related resources.
        initSwapChain();

        createSyncObjects();
    }

    std::shared_ptr<CommandBuffer> SwapChainVk::get_command_buffer() {
        auto command_buffer_vk = std::make_shared<CommandBufferVk>();
        command_buffer_vk->vk_command_buffer = commandBuffers[current_image];
        return command_buffer_vk;
    }

    bool SwapChainVk::acquire_image(uint32_t &image_index) {
        return acquireSwapChainImage(current_image);
    }

    void SwapChainVk::initSwapChain() {
        // Create a swap chain and corresponding swap chain images.
        createSwapChain();

        // Create views for swap chain images.
        createImageViews();

        createRenderPass();

        createFramebuffers();

        // Create a command buffer for each swap chain image.
        createCommandBuffers();
    }

    void SwapChainVk::recreateSwapChain() {
//        // Handling window minimization.
//        int width = 0, height = 0;
//        glfwGetFramebufferSize(window, &width, &height);
//        while (width == 0 || height == 0) {
//            glfwGetFramebufferSize(window, &width, &height);
//            glfwWaitEvents();
//        }
//
//        vkDeviceWaitIdle(device);
//
//        cleanupSwapChain();
//
//        initSwapChain();
//
//        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
    }

    void SwapChainVk::cleanupSwapChain() {
        auto device = driver->get_device();
        auto commandPool = driver->get_command_pool();
        // Command buffers contain swap chain related info, so we also need to free them here.
//        RenderServer::getSingleton().cleanupSwapChainRelatedResources();

        // Only command buffers are freed but not the pool.
        vkFreeCommandBuffers(device,
                             commandPool,
                             static_cast<uint32_t>(commandBuffers.size()),
                             commandBuffers.data());

        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto imageView: swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void SwapChainVk::cleanup() {
        auto device = driver->get_device();

        // Clean up swap chain related resources.
        cleanupSwapChain();

        // Clean up sync objects.
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
    }

    void SwapChainVk::createSwapChain() {
        auto device = driver->get_device();

        SwapChainSupportDetails swapChainSupport = platform->querySwapChainSupport(platform->physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = platform->chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = platform->chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = platform->chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = platform->surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices qfIndices = platform->findQueueFamilies(
                platform->physicalDevice);
        uint32_t queueFamilyIndices[] = {qfIndices.graphicsFamily.value(), qfIndices.presentFamily.value()};

        if (qfIndices.graphicsFamily != qfIndices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        // Create a swap chain.
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        // Get the number of presentable images for swap chain.
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);

        // Obtain the array of presentable images associated with a swap chain.
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void SwapChainVk::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = driver->createVkImageView(swapChainImages[i],
                                                               swapChainImageFormat,
                                                               VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }

    void SwapChainVk::createRenderPass() {
        render_pass = driver->create_render_pass(vk_to_texture_format(swapChainImageFormat));
    }

    void SwapChainVk::createFramebuffers() {
        auto device = driver->get_device();

        // FIXME: Swap chain framebuffers should not clean up its image.
        framebuffers.clear();

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            auto render_pass_vk = static_cast<RenderPassVk *>(render_pass.get());

            auto framebuffer_vk = std::make_shared<FramebufferVk>(
                    device,
                    render_pass_vk->get_vk_render_pass(),
                    extent.x,
                    extent.y,
                    swapChainImageViews[i]);

            framebuffers.push_back(framebuffer_vk);
        }
    }

    void SwapChainVk::createSyncObjects() {
        auto device = driver->get_device();

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Initialize it in the signaled state

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }
    }

    bool SwapChainVk::acquireSwapChainImage(uint32_t &imageIndex) {
        auto device = driver->get_device();

        // Wait for the frame to be finished.
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // Retrieve the index of the next available presentable image.
        VkResult result = vkAcquireNextImageKHR(device,
                                                swapChain,
                                                UINT64_MAX,
                                                imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE,
                                                &imageIndex);

        currentImage = imageIndex;

        // Recreate swap chains if necessary.
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return false;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        return true;
    }

    void SwapChainVk::createCommandBuffers() {
        auto device = driver->get_device();
        auto commandPool = driver->get_command_pool();

        commandBuffers.resize(framebuffers.size());

        // Allocate command buffers.
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void SwapChainVk::flush(uint32_t imageIndex) {
        auto device = driver->get_device();
        auto graphicsQueue = driver->get_graphics_queue();
        auto presentQueue = driver->get_present_queue();

        imageIndex = currentImage;

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        // Submit command buffer.
        // -------------------------------------
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        // The semaphores to signal after all commands in the buffer are finished.
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }
        // -------------------------------------

        // Queue an image for presentation after queueing all rendering commands and transitioning the image to the correct layout.
        // -------------------------------------
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // Specifies the semaphores to wait for before issuing the present request.
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        // Array of each swap chain’s presentable images.
        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
        // -------------------------------------

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || platform->framebufferResized) {
            platform->framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

#endif
