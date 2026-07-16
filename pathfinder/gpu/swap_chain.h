#pragma once

#include "../common/math/vec2.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

enum class PresentMode {
    Immediate, // V-Sync off
    Fifo,      // V-Sync on
    Mailbox,   // Fast V-Sync (triple buffering if available)
};

class SwapChain {
public:
    explicit SwapChain(const Vec2I size, PresentMode present_mode) : size_(size), present_mode_(present_mode) {}

    virtual ~SwapChain() = default;

    /// Swap chain framebuffer size.
    Vec2I size_;

    PresentMode present_mode_;

    /**
     * Get a swap chain render pass.
     * @return Render pass
     */
    virtual std::shared_ptr<RenderPass> get_render_pass() = 0;

    /**
     * Get current texture.
     * @return Texture
     */
    virtual std::shared_ptr<Texture> get_surface_texture() = 0;

    virtual TextureFormat get_surface_format() const = 0;

    /// Acquire current texture in the swap chain.
    virtual bool acquire_image() = 0;

    virtual void submit(const std::shared_ptr<CommandEncoder> &encoder) = 0;

    /**
     * Flush command buffers to the current image.
     */
    virtual void present() = 0;

    /// Clean up swap chain resources.
    virtual void destroy() = 0;
};

} // namespace Pathfinder
