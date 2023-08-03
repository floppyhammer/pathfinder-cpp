#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#include "../../common/global_macros.h"
#include "../swap_chain.h"
#include "command_buffer.h"
#include "framebuffer.h"
#include "render_pass.h"

#ifndef PATHFINDER_USE_VULKAN

namespace Pathfinder {

class SwapChainGl : public SwapChain {
    friend class DeviceGl;

public:
    #ifndef __ANDROID__

    SwapChainGl(Vec2I _size, GLFWwindow *_window) : SwapChain(_size) {
        window = _window;

        framebuffer = std::shared_ptr<FramebufferGl>(new FramebufferGl(size));

        command_buffer = std::shared_ptr<CommandBufferGl>(new CommandBufferGl());

        render_pass = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));
    }

    #endif

    inline std::shared_ptr<RenderPass> get_render_pass() override {
        return render_pass;
    };

    inline std::shared_ptr<Framebuffer> get_framebuffer() override {
        return framebuffer;
    }

    inline std::shared_ptr<CommandBuffer> get_command_buffer() override {
        return command_buffer;
    }

    inline bool acquire_image() override {
        return true;
    }

    inline void flush() override {
    #ifndef __ANDROID__
        glfwSwapBuffers(window);
    #endif
    }

    inline void cleanup() override {}

private:
    #ifndef __ANDROID__
    GLFWwindow *window;
    #endif
    std::shared_ptr<RenderPass> render_pass;
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<CommandBuffer> command_buffer;
};

} // namespace Pathfinder

#endif

#endif // PATHFINDER_GPU_SWAP_CHAIN_GL_H
