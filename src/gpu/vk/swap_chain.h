#ifndef PATHFINDER_GPU_SWAP_CHAIN_VK_H
#define PATHFINDER_GPU_SWAP_CHAIN_VK_H

#include "../swap_chain.h"
#include "framebuffer.h"
#include "platform.h"

#include <vector>

#ifdef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class SwapChainVk : public SwapChain {
        friend class DriverVk;

    public:
        SwapChainVk(uint32_t p_width,
                    uint32_t p_height,
                    PlatformVk *p_platform,
                    DriverVk *p_driver);

        std::shared_ptr<RenderPass> get_render_pass() override;

        inline std::shared_ptr<Framebuffer> get_framebuffer() override;

        std::shared_ptr<CommandBuffer> get_command_buffer() override;

        bool acquire_image() override;

    private:
        std::shared_ptr<RenderPass> render_pass;
        std::vector<std::shared_ptr<Framebuffer>> framebuffers;

        PlatformVk *platform;
        DriverVk *driver;

        void initSwapChain();

        VkSwapchainKHR swapChain;

        /// Swap chain images are allocated differently than normal images.
        /// Number of images doesn't necessarily equal to MAX_FRAMES_IN_FLIGHT (One is expected, the other is what we actually get considering device capacity).
        std::vector<VkImage> swapChainImages;

        /// VkImageView defines which part of VkImage to use.
        std::vector<VkImageView> swapChainImageViews;

        /// Store the format and extent we've chosen for the swap chain images.
        VkFormat swapChainImageFormat; // Default is VK_FORMAT_B8G8R8A8_SRGB.
        VkExtent2D swapChainExtent;

        /// VkFramebuffer + VkRenderPass defines the render target.
        /// Render pass defines which attachment will be written with colors.
        /// VkFramebuffer defines which VkImageView is to be which attachment.
//        std::vector<VkFramebuffer> swapChainFramebuffers;

        VkRenderPass renderPass;

        std::vector<VkCommandBuffer> commandBuffers;

        /// Each frame should have its own set of semaphores, so a list is used.
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;

        /// To perform CPU-GPU synchronization using fences.
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;

        /// To use the right pair of semaphores every time,
        /// we need to keep track of the current frame.
        size_t currentFrame = 0;

        uint32_t currentImage = 0;

        void recreateSwapChain();

        /**
         * What have been created in createSwapChainRelatedResources() have to be destroyed in here.
         */
        void cleanupSwapChain();

        /**
         * Vulkan does not use the idea of a "back buffer". So, we need a place to render into
         * before moving an image to viewing. This place is called the Swap Chain.
         *
         * In essence, the Swap Chain manages one or more image objects that
         * form a sequence of images that can be drawn into and then given to
         * the Surface to be presented to the user for viewing.
         */
        void createSwapChain();

        void createImageViews();

        /**
         * We need to tell Vulkan about the framebuffer attachments that
         * will be used while rendering. We need to specify how many
         * color and depth buffers there will be, how many samples to
         * use for each of them and how their contents should be
         * handled throughout the rendering operations. All of this
         * information is wrapped in a render pass object.
         * @note A render pass doesn't have viewport extent requirement, which means
         * we can bind framebuffers with any size to it.
         * @dependency Swap chain image format.
         */
        void createRenderPass();

        /**
         *
         * @dependency Swap chain extent, render pass, swap chain image views.
         */
        void createFramebuffers();

        void createSyncObjects();

        bool acquireSwapChainImage(uint32_t &imageIndex);

        /**
         * Set up command queues.
         * @dependency None.
         */
        void createCommandBuffers();

        void flush() override;

        void cleanup();
    };
}

#endif

#endif //PATHFINDER_GPU_SWAP_CHAIN_VK_H
