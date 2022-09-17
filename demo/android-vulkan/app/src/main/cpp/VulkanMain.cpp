// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VulkanMain.hpp"
#include "vulkan_wrapper.h"

#include "demo/common/app.h"

#include <android/log.h>

#include <cassert>
#include <cstring>
#include <vector>

// Android log function wrappers.
static const char *kTAG = "Vulkan-Tutorial05";
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

// Vulkan call wrapper.
#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    __android_log_print(ANDROID_LOG_ERROR, "Tutorial ",               \
                        "Vulkan error. File[%s], line[%d]", __FILE__, \
                        __LINE__);                                    \
    assert(false);                                                    \
  }

// Global Variables.
struct VulkanDeviceInfo {
    bool initialized_;

    VkInstance instance_;
    VkPhysicalDevice gpuDevice_;
    VkDevice device_;
    uint32_t queueFamilyIndex_;

    VkSurfaceKHR surface_;
    VkQueue queue_;
};
VulkanDeviceInfo device;

struct VulkanSwapchainInfo {
    VkSwapchainKHR swapchain_{};
    uint32_t swapchainLength_{};

    VkExtent2D displaySize_{};
    VkFormat displayFormat_{};

    // array of frame buffers and views
    std::vector<VkImage> displayImages_;
    std::vector<VkImageView> displayViews_;
    std::vector<VkFramebuffer> framebuffers_;
};
VulkanSwapchainInfo swapchain;

struct VulkanRenderInfo {
    VkRenderPass renderPass_;
    VkCommandPool cmdPool_;
    VkCommandBuffer *cmdBuffer_;
    uint32_t cmdBufferLen_;
    VkSemaphore semaphore_;
    VkFence fence_;
};
VulkanRenderInfo render;

// Android Native App pointer.
android_app *androidAppCtx = nullptr;

std::shared_ptr<App> pathfinder_app;
std::shared_ptr<Pathfinder::Driver> driver;

/*
 * setImageLayout():
 *    Helper function to transition color buffer layout
 */
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image,
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStages,
                    VkPipelineStageFlags destStages);

// Create vulkan device
void CreateVulkanDevice(ANativeWindow *platformWindow,
                        VkApplicationInfo *appInfo) {
    std::vector<const char *> instance_extensions;
    std::vector<const char *> device_extensions;

    instance_extensions.push_back("VK_KHR_surface");
    instance_extensions.push_back("VK_KHR_android_surface");

    device_extensions.push_back("VK_KHR_swapchain");

    // **********************************************************
    // Create the Vulkan instance
    VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount =
            static_cast<uint32_t>(instance_extensions.size()),
            .ppEnabledExtensionNames = instance_extensions.data(),
    };
    CALL_VK(vkCreateInstance(&instanceCreateInfo, nullptr, &device.instance_));
    VkAndroidSurfaceCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = platformWindow};

    CALL_VK(vkCreateAndroidSurfaceKHR(device.instance_, &createInfo, nullptr,
                                      &device.surface_));
    // Find one GPU to use:
    // On Android, every GPU device is equal -- supporting graphics/compute/present.
    // For this sample, we use the very first GPU device found on the system.
    uint32_t gpuCount = 0;
    CALL_VK(vkEnumeratePhysicalDevices(device.instance_, &gpuCount, nullptr));
    VkPhysicalDevice tmpGpus[gpuCount];
    CALL_VK(vkEnumeratePhysicalDevices(device.instance_, &gpuCount, tmpGpus));

    // Pick up the first GPU device.
    device.gpuDevice_ = tmpGpus[0];

    // Find a GFX queue family.
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount,
                                             nullptr);
    assert(queueFamilyCount);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount,
                                             queueFamilyProperties.data());

    uint32_t queueFamilyIndex;
    for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount;
         queueFamilyIndex++) {
        if (queueFamilyProperties[queueFamilyIndex].queueFlags &
            VK_QUEUE_GRAPHICS_BIT) {
            break;
        }
    }
    assert(queueFamilyIndex < queueFamilyCount);
    device.queueFamilyIndex_ = queueFamilyIndex;

    // Create a logical device (vulkan device)
    float priorities[] = {
            1.0f,
    };
    VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = priorities,
    };

    VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
            .pEnabledFeatures = nullptr,
    };

    CALL_VK(vkCreateDevice(device.gpuDevice_, &deviceCreateInfo, nullptr,
                           &device.device_));

    vkGetDeviceQueue(device.device_, device.queueFamilyIndex_, 0, &device.queue_);
}

void CreateSwapChain() {
    LOGI("->createSwapChain");

    // **********************************************************
    // Get the surface capabilities because:
    //   - It contains the minimal and max length of the chain, we will need it
    //   - It's necessary to query the supported surface format (R8G8B8A8 for
    //   instance ...)
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_,
                                              &surfaceCapabilities);
    // Query the list of supported surface format and choose one we like
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_,
                                         &formatCount, nullptr);
    auto *formats = new VkSurfaceFormatKHR[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_,
                                         &formatCount, formats);
    LOGI("Got %d formats", formatCount);

    uint32_t chosenFormat;
    for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
        if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
    }
    assert(chosenFormat < formatCount);

    swapchain.displaySize_ = surfaceCapabilities.currentExtent;
    swapchain.displayFormat_ = formats[chosenFormat].format;

    VkSurfaceCapabilitiesKHR surfaceCap;
    CALL_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_,
                                                      device.surface_, &surfaceCap));
    assert(surfaceCap.supportedCompositeAlpha | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);

    // **********************************************************
    // Create a swap chain (here we choose the minimum available number of surface
    // in the chain)
    VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .surface = device.surface_,
            .minImageCount = surfaceCapabilities.minImageCount,
            .imageFormat = formats[chosenFormat].format,
            .imageColorSpace = formats[chosenFormat].colorSpace,
            .imageExtent = surfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &device.queueFamilyIndex_,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = VK_FALSE,
            .oldSwapchain = VK_NULL_HANDLE,
    };
    CALL_VK(vkCreateSwapchainKHR(device.device_, &swapchainCreateInfo, nullptr,
                                 &swapchain.swapchain_));

    // Get the length of the created swap chain
    CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
                                    &swapchain.swapchainLength_, nullptr));
    delete[] formats;
    LOGI("<-createSwapChain");
}

void DeleteSwapChain() {
    for (int i = 0; i < swapchain.swapchainLength_; i++) {
        vkDestroyFramebuffer(device.device_, swapchain.framebuffers_[i], nullptr);
        vkDestroyImageView(device.device_, swapchain.displayViews_[i], nullptr);
    }
    vkDestroySwapchainKHR(device.device_, swapchain.swapchain_, nullptr);
}

void CreateFrameBuffers(VkRenderPass &renderPass,
                        VkImageView depthView = VK_NULL_HANDLE) {
    // query display attachment to swapchain
    uint32_t SwapchainImagesCount = 0;
    CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
                                    &SwapchainImagesCount, nullptr));
    swapchain.displayImages_.resize(SwapchainImagesCount);
    CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
                                    &SwapchainImagesCount,
                                    swapchain.displayImages_.data()));

    // create image view for each swapchain image
    swapchain.displayViews_.resize(SwapchainImagesCount);
    for (uint32_t i = 0; i < SwapchainImagesCount; i++) {
        VkImageViewCreateInfo viewCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = swapchain.displayImages_[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchain.displayFormat_,
                .components =
                        {
                                .r = VK_COMPONENT_SWIZZLE_R,
                                .g = VK_COMPONENT_SWIZZLE_G,
                                .b = VK_COMPONENT_SWIZZLE_B,
                                .a = VK_COMPONENT_SWIZZLE_A,
                        },
                .subresourceRange =
                        {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = 1,
                        },
        };
        CALL_VK(vkCreateImageView(device.device_, &viewCreateInfo, nullptr,
                                  &swapchain.displayViews_[i]));
    }
    // create a framebuffer from each swapchain image
    swapchain.framebuffers_.resize(swapchain.swapchainLength_);
    for (uint32_t i = 0; i < swapchain.swapchainLength_; i++) {
        VkImageView attachments[2] = {
                swapchain.displayViews_[i], depthView,
        };
        VkFramebufferCreateInfo fbCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .renderPass = renderPass,
                .attachmentCount = 1,  // 2 if using depth
                .pAttachments = attachments,
                .width = static_cast<uint32_t>(swapchain.displaySize_.width),
                .height = static_cast<uint32_t>(swapchain.displaySize_.height),
                .layers = 1,
        };
        fbCreateInfo.attachmentCount = (depthView == VK_NULL_HANDLE ? 1 : 2);

        CALL_VK(vkCreateFramebuffer(device.device_, &fbCreateInfo, nullptr,
                                    &swapchain.framebuffers_[i]));
    }
}


// InitVulkan:
//   Initialize Vulkan Context when android application window is created
//   upon return, vulkan is ready to draw frames
bool InitVulkan(android_app *app) {
    androidAppCtx = app;

    if (!InitVulkan()) {
        LOGW("Vulkan is unavailable, install vulkan and re-start");
        return false;
    }

    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "pathfinder_vulkan_demo",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "pathfinder",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_MAKE_VERSION(1, 1, 0),
    };

    // create a device
    CreateVulkanDevice(app->window, &appInfo);

    CreateSwapChain();

    // -----------------------------------------------------------------
    // Create render pass
    VkAttachmentDescription attachmentDescriptions{
            .format = swapchain.displayFormat_,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference colourReference = {
            .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colourReference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
    };
    VkRenderPassCreateInfo renderPassCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .attachmentCount = 1,
            .pAttachments = &attachmentDescriptions,
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 0,
            .pDependencies = nullptr,
    };
    CALL_VK(vkCreateRenderPass(device.device_, &renderPassCreateInfo, nullptr,
                               &render.renderPass_));

    // -----------------------------------------------------------------
    // Create 2 frame buffers.
    CreateFrameBuffers(render.renderPass_);

    // -----------------------------------------------
    // Create a pool of command buffers to allocate command buffer from
    VkCommandPoolCreateInfo cmdPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = device.queueFamilyIndex_,
    };
    CALL_VK(vkCreateCommandPool(device.device_, &cmdPoolCreateInfo, nullptr,
                                &render.cmdPool_));

    // Record a command buffer that just clear the screen
    // 1 command buffer draw in 1 framebuffer
    // In our case we need 2 command as we have 2 framebuffer
    render.cmdBufferLen_ = swapchain.swapchainLength_;
    render.cmdBuffer_ = new VkCommandBuffer[swapchain.swapchainLength_];
    VkCommandBufferAllocateInfo cmdBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = render.cmdPool_,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = render.cmdBufferLen_,
    };
    CALL_VK(vkAllocateCommandBuffers(device.device_, &cmdBufferCreateInfo,
                                     render.cmdBuffer_));

    // Pathfinder initialization.
    // -----------------------------
    AAsset *file = AAssetManager_open(androidAppCtx->activity->assetManager,
                                      "assets/tiger.svg",
                                      AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);

    std::vector<char> fileContent(fileLength);

    AAsset_read(file, fileContent.data(), fileLength);
    AAsset_close(file);

    driver = std::make_shared<Pathfinder::DriverVk>(
            device.device_,
            device.gpuDevice_,
            device.queue_,
            device.queue_,
            render.cmdPool_);

    pathfinder_app = std::make_shared<App>(driver,
                                           swapchain.displaySize_.width,
                                           swapchain.displaySize_.height,
                                           fileContent);
    // -----------------------------

    for (int bufferIndex = 0; bufferIndex < swapchain.swapchainLength_; bufferIndex++) {
        // We start by creating and declare the "beginning" our command buffer
        VkCommandBufferBeginInfo cmdBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pInheritanceInfo = nullptr,
        };
        CALL_VK(vkBeginCommandBuffer(render.cmdBuffer_[bufferIndex],
                                     &cmdBufferBeginInfo));
        // transition the display image to color attachment layout
        setImageLayout(render.cmdBuffer_[bufferIndex],
                       swapchain.displayImages_[bufferIndex],
                       VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Now we start a renderpass. Any draw command has to be recorded in a
        // renderpass
        VkClearValue clearVals{.color {.float32 {0.0f, 0.34f, 0.90f, 1.0f}}};
        VkRenderPassBeginInfo renderPassBeginInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = render.renderPass_,
                .framebuffer = swapchain.framebuffers_[bufferIndex],
                .renderArea = {.offset {.x = 0, .y = 0,},
                        .extent = swapchain.displaySize_},
                .clearValueCount = 1,
                .pClearValues = &clearVals};
        vkCmdBeginRenderPass(render.cmdBuffer_[bufferIndex], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        // Drawing goes here.

        vkCmdEndRenderPass(render.cmdBuffer_[bufferIndex]);

        CALL_VK(vkEndCommandBuffer(render.cmdBuffer_[bufferIndex]));
    }

    // We need to create a fence to be able, in the main loop, to wait for our
    // draw command(s) to finish before swapping the framebuffers
    VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    CALL_VK(vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render.fence_));

    // We need to create a semaphore to be able to wait, in the main loop, for our
    // framebuffer to be available for us before drawing.
    VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    CALL_VK(vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr, &render.semaphore_));

    device.initialized_ = true;
    return true;
}

// Native app poll to see if we are ready to draw.
bool IsVulkanReady() { return device.initialized_; }

void DeleteVulkan() {
    vkFreeCommandBuffers(device.device_, render.cmdPool_, render.cmdBufferLen_,
                         render.cmdBuffer_);
    delete[] render.cmdBuffer_;

    vkDestroyCommandPool(device.device_, render.cmdPool_, nullptr);
    vkDestroyRenderPass(device.device_, render.renderPass_, nullptr);

    DeleteSwapChain();

    // Clean up Pathfinder here.

    vkDestroyDevice(device.device_, nullptr);
    vkDestroyInstance(device.instance_, nullptr);

    device.initialized_ = false;
}

// Draw one frame.
bool VulkanDrawFrame() {
    uint32_t nextIndex;
    // Get the framebuffer index we should draw in.
    CALL_VK(vkAcquireNextImageKHR(device.device_, swapchain.swapchain_,
                                  UINT64_MAX, render.semaphore_, VK_NULL_HANDLE,
                                  &nextIndex));
    CALL_VK(vkResetFences(device.device_, 1, &render.fence_));

    VkPipelineStageFlags waitStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render.semaphore_,
            .pWaitDstStageMask = &waitStageMask,
            .commandBufferCount = 1,
            .pCommandBuffers = &render.cmdBuffer_[nextIndex],
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr
    };
    CALL_VK(vkQueueSubmit(device.queue_, 1, &submit_info, render.fence_));
    CALL_VK(vkWaitForFences(device.device_, 1, &render.fence_, VK_TRUE, 100000000));

    VkResult result;
    VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &swapchain.swapchain_,
            .pImageIndices = &nextIndex,
            .pResults = &result,
    };
    vkQueuePresentKHR(device.queue_, &presentInfo);

    return true;
}

// Helper function to transition image layout.
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image,
                    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                    VkPipelineStageFlags srcStages,
                    VkPipelineStageFlags destStages) {
    VkImageMemoryBarrier imageMemoryBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldImageLayout,
            .newLayout = newImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange =
                    {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                    },
    };

    switch (oldImageLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        default:
            break;
    }

    switch (newImageLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            break;

        default:
            break;
    }

    vkCmdPipelineBarrier(cmdBuffer, srcStages, destStages, 0, 0, nullptr, 0, nullptr, 1,
                         &imageMemoryBarrier);
}
