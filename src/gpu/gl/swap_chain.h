#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#include "../swap_chain.h"
#include "framebuffer.h"
#include "command_buffer.h"
#include "render_pass.h"
#include "../../common/global_macros.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {
    class SwapChainGl : public SwapChain {
        friend class DriverGl;

    public:
        SwapChainGl(uint32_t p_width, uint32_t p_height, GLFWwindow *p_window) : SwapChain(p_width, p_height) {
            framebuffer = std::make_shared<FramebufferGl>(p_width, p_height);
            window = p_window;

            command_buffer = std::make_shared<CommandBufferGl>();

            render_pass = std::make_shared<RenderPassGl>();
        }

        inline std::shared_ptr<RenderPass> get_render_pass() override {
            return render_pass;
        };

        inline std::shared_ptr<Framebuffer> get_framebuffer(uint32_t image_index) override {
            return framebuffer;
        }

        inline std::shared_ptr<CommandBuffer> get_command_buffer() override {
            return command_buffer;
        }

        inline bool acquire_image(uint32_t &image_index) override { return true; }

        inline void flush(uint32_t imageIndex) override { glfwSwapBuffers(window); };

    private:
        GLFWwindow *window;
        std::shared_ptr<RenderPass> render_pass;
        std::shared_ptr<Framebuffer> framebuffer;
        std::shared_ptr<CommandBuffer> command_buffer;
    };
}

#endif

#endif //PATHFINDER_GPU_SWAP_CHAIN_GL_H
