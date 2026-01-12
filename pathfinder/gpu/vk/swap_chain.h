#ifndef PATHFINDER_GPU_SWAP_CHAIN_VK_H
#define PATHFINDER_GPU_SWAP_CHAIN_VK_H

#include <vector>

#include "../swap_chain.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "window.h"

namespace Pathfinder {

class WindowBuilderVk;

/// How many frames should be processed concurrently by CPU.
/// Typical Value: Usually 2. This provides a balance: the CPU can work on the next frame while the GPU finishes the
/// current one, but it doesn't add too much "input lag" (latency).
constexpr int MAX_FRAMES_IN_FLIGHT = 2; // (CPU-to-GPU Buffering)

class SwapChainVk : public SwapChain {
    friend class DeviceVk;
    friend class QueueVk;

public:
    SwapChainVk(Vec2I size, WindowVk *window, DeviceVk *device);

    std::shared_ptr<RenderPass> get_render_pass() override;

    std::shared_ptr<Texture> get_surface_texture() override;

    TextureFormat get_surface_format() const override;

    bool acquire_image() override;

    void submit(const std::shared_ptr<CommandEncoder> &encoder) override;

private:
    WindowVk *window_{};
    DeviceVk *device_{};

    VkSwapchainKHR vk_swapchain_{};

    std::shared_ptr<RenderPass> render_pass_;
    std::vector<std::shared_ptr<Framebuffer>> framebuffers_;

    /// It handles the transition from the GPU to the display. You need enough images so that while one is being
    /// displayed on the screen, another can be written to by the GPU, and potentially a third can be "queued up" for
    /// the next refresh.
    /// Typical Value: Usually 3 (for Triple Buffering) or 2 (for Double Buffering).
    std::vector<VkImage> swapchain_images_;

    uint32_t image_count_; // (GPU-to-Display Buffering)

    /// VkImageView defines which part of VkImage to use.
    std::vector<VkImageView> swapchain_image_views_;

    /// The format for the swap chain images.
    /// Default will be VK_FORMAT_B8G8R8A8_SRGB.
    VkFormat swapchain_image_format_{};

    // --------------------------------------
    /// To use the right pair of semaphores every time,
    /// we need to keep track of the current frame.
    size_t current_frame_ = 0; // Index for MAX_FRAMES_IN_FLIGHT

    /// Used to tell the GPU "the image is ready to be drawn on."
    std::vector<VkSemaphore> image_available_semaphores_; // Check before acquiring an image.

    /// This ensures the CPU doesn't overwrite the command buffer while the GPU is still reading it.
    std::vector<VkFence> in_flight_fences_;
    // --------------------------------------

    // --------------------------------------
    uint32_t image_index_ = 0; // Index for image_count_

    /// (Ideally) these should be per-image to ensure the presentation engine knows exactly when a specific image is
    /// ready.
    std::vector<VkSemaphore> render_finished_semaphores_; // Check before presenting an image.

    /// This ensures you don't draw into an image that is still being presented or processed by an older frame.
    std::vector<VkFence> images_in_flight_;
    // --------------------------------------

    std::vector<std::shared_ptr<CommandEncoder>> encoders_in_flight_;

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
