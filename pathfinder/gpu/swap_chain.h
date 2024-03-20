#ifndef PATHFINDER_GPU_SWAP_CHAIN_H
#define PATHFINDER_GPU_SWAP_CHAIN_H

#include "../common/math/vec2.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

class SwapChain {
public:
    explicit SwapChain(Vec2I size) : size_(size) {}

    virtual ~SwapChain() = default;

    /// Swap chain framebuffer size.
    Vec2I size_;

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

    /**
     * Flush command buffers to the current image.
     */
    virtual void present() = 0;

    /// Clean up swap chain resources.
    virtual void destroy() = 0;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_H
