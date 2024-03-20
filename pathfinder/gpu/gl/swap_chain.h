#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#include "../../common/global_macros.h"
#include "../swap_chain.h"
#include "base.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

class SwapChainGl : public SwapChain {
    friend class DeviceGl;

public:
#ifndef __ANDROID__
    SwapChainGl(Vec2I size, GLFWwindow *window_handle) : SwapChain(size) {
        glfw_window_ = window_handle;

        command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

        render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));
    }
#else
    SwapChainGl(Vec2I _size) : SwapChain(_size) {
        command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

        render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));
    }
#endif

    std::shared_ptr<RenderPass> get_render_pass() override {
        return render_pass_;
    }

    std::shared_ptr<Texture> get_surface_texture() override {
        return nullptr;
    }

    TextureFormat get_surface_format() const override {
        return TextureFormat::Rgba8Unorm;
    }

    bool acquire_image() override {
#ifndef __ANDROID__
        glfwMakeContextCurrent(glfw_window_);
#endif
        return true;
    }

    void present() override {
#ifndef __ANDROID__
        glfwSwapBuffers(glfw_window_);
#endif
    }

    void destroy() override {}

private:
#ifndef __ANDROID__
    GLFWwindow *glfw_window_;
#endif

    std::shared_ptr<RenderPass> render_pass_;
    std::shared_ptr<CommandEncoder> command_encoder_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_GL_H
