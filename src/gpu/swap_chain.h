#ifndef PATHFINDER_GPU_SWAP_CHAIN_H
#define PATHFINDER_GPU_SWAP_CHAIN_H

#include "render_pass.h"
#include "framebuffer.h"
#include "command_buffer.h"
#include "../common/math/vec2.h"

namespace Pathfinder {
    class SwapChain {
    public:
        SwapChain(uint32_t p_width, uint32_t p_height) : extent({p_width, p_height}) {}

        Vec2<uint32_t> extent;

        uint32_t current_image;

        virtual std::shared_ptr<RenderPass> get_render_pass() = 0;

        virtual std::shared_ptr<Framebuffer> get_framebuffer(uint32_t image_index) = 0;

        virtual std::shared_ptr<CommandBuffer> get_command_buffer() = 0;

        virtual bool acquire_image(uint32_t &image_index) = 0;

        /**
         * Flush command buffers to the image with the index.
         * @param imageIndex Which image in the swap chain should be used.
         */
        virtual void flush(uint32_t imageIndex) = 0;
    };
}

#endif //PATHFINDER_GPU_SWAP_CHAIN_H
