#ifndef PATHFINDER_GPU_SWAP_CHAIN_VK_H
#define PATHFINDER_GPU_SWAP_CHAIN_VK_H

#include "../swap_chain.h"
#include "framebuffer.h"

#include <vector>

namespace Pathfinder {
    class SwapChainVk : public SwapChain {
        friend class DriverVk;
    public:
        SwapChainVk(uint32_t p_width, uint32_t p_height) {
        }

        std::shared_ptr<RenderPass> get_render_pass() override {
            return render_pass;
        };

        inline std::shared_ptr<Framebuffer> get_framebuffer(uint32_t image_index) override {
            return framebuffers[image_index];
        }

    private:
        std::shared_ptr<RenderPass> render_pass;
        std::vector<std::shared_ptr<Framebuffer>> framebuffers;
    };
}

#endif //PATHFINDER_GPU_SWAP_CHAIN_VK_H
