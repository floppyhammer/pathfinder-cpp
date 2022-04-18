#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#include "../swap_chain.h"
#include "framebuffer.h"

namespace Pathfinder {
    class SwapChainGl : public SwapChain {
        friend class DeviceGl;
    public:
        SwapChainGl(uint32_t p_width, uint32_t p_height) {
            framebuffer = std::make_shared<FramebufferGl>(p_width, p_height);
        };

        inline std::shared_ptr<Framebuffer> get_framebuffer(uint32_t image_index) override {
            return framebuffer;
        }
    };
}

#endif //PATHFINDER_GPU_SWAP_CHAIN_GL_H
