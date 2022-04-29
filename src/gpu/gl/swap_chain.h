#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#include "../swap_chain.h"
#include "framebuffer.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class SwapChainGl : public SwapChain {
        friend class DriverGl;
    public:
        SwapChainGl(uint32_t p_width, uint32_t p_height) {
            framebuffer = std::make_shared<FramebufferGl>(p_width, p_height);
        }

        std::shared_ptr<RenderPass> get_render_pass() override {
            return render_pass;
        };

        inline std::shared_ptr<Framebuffer> get_framebuffer(uint32_t image_index) override {
            return framebuffer;
        }

        void flush(uint32_t imageIndex) override {glfwSwapBuffers()};

    private:
        std::shared_ptr<RenderPass> render_pass;
        std::shared_ptr<Framebuffer> framebuffer;
    };
}

#endif

#endif //PATHFINDER_GPU_SWAP_CHAIN_GL_H
