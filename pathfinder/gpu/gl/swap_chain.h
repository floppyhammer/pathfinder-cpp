#ifndef PATHFINDER_GPU_SWAP_CHAIN_GL_H
#define PATHFINDER_GPU_SWAP_CHAIN_GL_H

#ifdef __ANDROID__
    #include <EGL/egl.h>
#endif

#include "../../common/global_macros.h"
#include "../swap_chain.h"
#include "base.h"
#include "command_encoder.h"
#include "framebuffer.h"
#include "render_pass.h"

namespace Pathfinder {

class SwapChainGl : public SwapChain {
    friend class DeviceGl;
    friend class CommandEncoderGl;

public:
#ifndef __ANDROID__
    SwapChainGl(Vec2I size, GLFWwindow *window_handle) : SwapChain(size) {
        glfw_window_ = window_handle;

        command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

        render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));
    }
#else
    SwapChainGl(Vec2I _size, EGLDisplay egl_display, EGLSurface egl_surface, EGLContext egl_context)
        : SwapChain(_size) {
        command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

        render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));

        egl_display_ = egl_display;
        egl_surface_ = egl_surface;
        egl_context_ = egl_context;
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
#else
        // This is required for GL on Android, otherwise GPU memory leak occurs.
        eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
#endif

        // Disable VSync (for performance comparison).
        // glfwSwapInterval(0);

        return true;
    }

    void submit(const std::shared_ptr<CommandEncoder> &encoder) override {
        if (encoder->submitted_) {
            Logger::error("Attempted to submit an encoder that's already been submitted!");
            return;
        }

        encoder->submitted_ = true;

        encoder->finish();
    }

    void present() override {
#ifndef __ANDROID__
        glfwSwapBuffers(glfw_window_);
#else
        eglSwapBuffers(egl_display_, egl_surface_);
#endif
    }

    void destroy() override {}

private:
#ifndef __ANDROID__
    GLFWwindow *glfw_window_;
#else
    EGLDisplay egl_display_;
    EGLSurface egl_surface_;
    EGLContext egl_context_;
#endif

    std::shared_ptr<RenderPass> render_pass_;
    std::shared_ptr<CommandEncoder> command_encoder_;
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SWAP_CHAIN_GL_H
