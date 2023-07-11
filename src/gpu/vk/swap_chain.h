#ifndef PATHFINDER_GPU_SWAP_CHAIN_VK_H
#define PATHFINDER_GPU_SWAP_CHAIN_VK_H

#include <vector>

#include "../swap_chain.h"
#include "command_buffer.h"
#include "framebuffer.h"
#include "window.h"

#ifdef PATHFINDER_USE_VULKAN

    #if defined(WIN32) || defined(__linux__) || defined(__APPLE__)

namespace Pathfinder {

/// How many frames should be processed concurrently.
/// NOTE: Swapchain image count doesn't necessarily equal to this (One is expected, the other is what we
/// actually get considering device capacity).
const int MAX_FRAMES_IN_FLIGHT = 2;

class SwapChainVk : public SwapChain {
    friend class DeviceVk;

public:
    SwapChainVk(Vec2I _size, WindowVk *_window, DeviceVk *_device);

    std::shared_ptr<RenderPass> get_render_pass() override;

    std::shared_ptr<Framebuffer> get_framebuffer() override;

    std::shared_ptr<CommandBuffer> get_command_buffer() override;

    bool acquire_image() override;

private:
    std::shared_ptr<RenderPass> render_pass;
    std::vector<std::shared_ptr<Framebuffer>> framebuffers;

    WindowVk *window{};
    DeviceVk *device_vk{};

    VkSwapchainKHR swapchain{};

    /// Swap chain images are allocated differently than normal images.
    /// Number of images doesn't necessarily equal to MAX_FRAMES_IN_FLIGHT (One is expected, the other is what we
    /// actually get considering device capacity).
    std::vector<VkImage> swapchain_images;

    /// VkImageView defines which part of VkImage to use.
    std::vector<VkImageView> swapchain_image_views;

    /// The format for the swap chain images.
    /// Default will be VK_FORMAT_B8G8R8A8_SRGB.
    VkFormat swapchain_image_format{};

    std::vector<VkCommandBuffer> command_buffers;

    /// Each frame should have its own set of semaphores, so a list is used.
    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;

    /// To perform CPU-GPU synchronization using fences.
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;

    /// To use the right pair of semaphores every time,
    /// we need to keep track of the current frame.
    size_t current_frame = 0;

    uint32_t image_index = 0;

private:
    void init_swapchain();

    void recreate_swapchain();

    void cleanup_swapchain();

    /**
     * Vulkan does not use the idea of a "back buffer". So, we need a place to render into
     * before moving an image to viewing. This place is called the Swap Chain.
     *
     * In essence, the Swap Chain manages one or more image objects that
     * form a sequence of images that can be drawn into and then given to
     * the Surface to be presented to the user for viewing.
     */
    void create_swapchain();

    void create_image_views();

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
    void create_render_pass();

    /**
     *
     * @dependency Swap chain extent, render pass, swap chain image views.
     */
    void create_framebuffers();

    void create_sync_objects();

    /**
     * Allocate command buffers in the pool.
     * @dependency Command pool.
     */
    void create_command_buffers();

    void flush() override;

    void cleanup() override;
};

} // namespace Pathfinder

    #endif

#endif

#endif // PATHFINDER_GPU_SWAP_CHAIN_VK_H
