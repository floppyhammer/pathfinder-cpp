#ifndef PATHFINDER_GPU_SWAP_CHAIN_H
#define PATHFINDER_GPU_SWAP_CHAIN_H

#include "../common/math/vec2.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

class SwapChain {
public:
    explicit SwapChain(Vec2I _size) : size(_size) {}

    virtual ~SwapChain() = default;

    /// Swap chain framebuffer size.
    Vec2I size;

    /**
     * Get a swap chain render pass.
     * @return Render pass
     */
    virtual std::shared_ptr<RenderPass> get_render_pass() = 0;

    /**
     * Get current framebuffer.
     * @return Framebuffer
     */
    virtual std::shared_ptr<Framebuffer> get_framebuffer() = 0;

    /// Acquire current texture in the swap chain.
    virtual bool acquire_image() = 0;

    /**
     * Flush command buffers to the current image.
     */
    virtual void present() = 0;

    /// Clean up swap chain resources.
    virtual void cleanup() = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_H
