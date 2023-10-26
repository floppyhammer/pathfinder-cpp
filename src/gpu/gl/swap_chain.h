#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#include "../../common/global_macros.h"
#include "../swap_chain.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class SwapChainGl : public SwapChain {
    friend class DeviceGl;

public:
    #ifndef __ANDROID__

    SwapChainGl(Vec2I _size, GLFWwindow *window_handle) : SwapChain(_size) {
        glfw_window = window_handle;

        framebuffer = std::shared_ptr<FramebufferGl>(new FramebufferGl(size));

        command_encoder = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

        render_pass = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));
    }

    #endif

    inline std::shared_ptr<RenderPass> get_render_pass() override {
        return render_pass;
    };

    inline std::shared_ptr<Framebuffer> get_framebuffer() override {
        return framebuffer;
    }

    inline bool acquire_image() override {
    #ifndef __ANDROID__
        glfwMakeContextCurrent(glfw_window);
    #endif
        return true;
    }

    inline void present() override {
    #ifndef __ANDROID__
        glfwSwapBuffers(glfw_window);
    #endif
    }

    inline void cleanup() override {}

private:
    #ifndef __ANDROID__
    GLFWwindow *glfw_window;
    #endif

    std::shared_ptr<RenderPass> render_pass;
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<CommandEncoder> command_encoder;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_SWAP_CHAIN_GL_H
