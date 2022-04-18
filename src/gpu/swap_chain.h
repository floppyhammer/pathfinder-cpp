#ifndef PATHFINDER_GPU_SWAP_CHAIN_H
#define PATHFINDER_GPU_SWAP_CHAIN_H

#include "framebuffer.h"
#include "../common/math/vec2.h"

namespace Pathfinder {
    class SwapChain {
    public:
        Vec2<uint32_t> extent;

        virtual std::shared_ptr<Framebuffer> get_framebuffer(uint32_t image_index) = 0;

    protected:
        std::shared_ptr<Framebuffer> framebuffer;
    };
}

#endif //PATHFINDER_GPU_SWAP_CHAIN_H
