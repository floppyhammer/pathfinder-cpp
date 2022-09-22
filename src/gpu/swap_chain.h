#ifndef PATHFINDER_GPU_SWAP_CHAIN_H
#define PATHFINDER_GPU_SWAP_CHAIN_H

#include "../common/math/vec2.h"
#include "command_buffer.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {
class SwapChain {
public:
    SwapChain(uint32_t width, uint32_t height) : extent({width, height}) {
    }

    /// Swap chain framebuffer size.
    Vec2<uint32_t> extent;

    /// Currently available image in the chain.
    uint32_t current_image{};

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

    /**
     * Get current command buffer.
     * @return Command buffer
     */
    virtual std::shared_ptr<CommandBuffer> get_command_buffer() = 0;

    /**
     * Acquire an image in the swap chain.
     * @return Successful
     */
    virtual bool acquire_image() = 0;

    /**
     * Flush command buffers to the current image.
     */
    virtual void flush() = 0;

    /// Clean up swap chain resources.
    virtual void cleanup() = 0;
};
} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_H
