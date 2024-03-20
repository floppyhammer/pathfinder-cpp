#ifndef PATHFINDER_GPU_SWAP_CHAIN_VK_H
#define PATHFINDER_GPU_SWAP_CHAIN_VK_H

#include <vector>

#include "../swap_chain.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "window.h"

namespace Pathfinder {

class WindowBuilderVk;

/// How many frames should be processed concurrently.
/// NOTE: Swapchain image count doesn't necessarily equal to this (One is expected, the other is what we
/// actually get considering device capacity).
const int MAX_FRAMES_IN_FLIGHT = 2;

class SwapChainVk : public SwapChain {
    friend class DeviceVk;
    friend class QueueVk;

public:
    SwapChainVk(Vec2I size, WindowVk *window, DeviceVk *device);

    std::shared_ptr<RenderPass> get_render_pass() override;

    std::shared_ptr<Texture> get_surface_texture() override;

    TextureFormat get_surface_format() const override;

    bool acquire_image() override;

private:
    WindowVk *window_{};
    DeviceVk *device_{};

    VkSwapchainKHR vk_swapchain_{};

    std::shared_ptr<RenderPass> render_pass_;
    std::vector<std::shared_ptr<Framebuffer>> framebuffers_;

    /// Swap chain images are allocated differently than normal images.
    /// Number of images doesn't necessarily equal to MAX_FRAMES_IN_FLIGHT (One is expected, the other is what we
    /// actually get considering device capacity).
    std::vector<VkImage> swapchain_images_;

    /// VkImageView defines which part of VkImage to use.
    std::vector<VkImageView> swapchain_image_views_;

    /// The format for the swap chain images.
    /// Default will be VK_FORMAT_B8G8R8A8_SRGB.
    VkFormat swapchain_image_format_{};

    /// Each frame should have its own set of semaphores, so a list is used.
    std::vector<VkSemaphore> image_available_semaphores_; // Check before acquiring an image.
    std::vector<VkSemaphore> render_finished_semaphores_; // Check before presenting an image.

    /// To perform CPU-GPU synchronization using fences.
    std::vector<VkFence> in_flight_fences_;
    std::vector<VkFence> images_in_flight_;

    /// To use the right pair of semaphores every time,
    /// we need to keep track of the current frame.
    size_t current_frame_ = 0;

    uint32_t image_index_ = 0;

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
    void create_swapchain(VkPhysicalDevice physical_device);

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

    void create_sync_objects();

    void flush(const std::shared_ptr<CommandEncoder> &encoder);

    void present() override;

    void destroy() override;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_VK_H
