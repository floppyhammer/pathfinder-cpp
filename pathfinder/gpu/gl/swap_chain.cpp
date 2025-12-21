#include "swap_chain.h"

#include "render_pass.h"

namespace Pathfinder {

#ifndef __ANDROID__
SwapChainGl::SwapChainGl(const Vec2I size, void *window_handle) : SwapChain(size) {
    glfw_window_ = window_handle;

    command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

    render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));
}
#else
SwapChainGl::SwapChainGl(Vec2I size, EGLDisplay egl_display, EGLSurface egl_surface, EGLContext egl_context)
    : SwapChain(size) {
    command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

    render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear));

    egl_display_ = egl_display;
    egl_surface_ = egl_surface;
    egl_context_ = egl_context;
}
#endif

bool SwapChainGl::acquire_image() {
#ifndef __ANDROID__
    glfwMakeContextCurrent((GLFWwindow *)glfw_window_);
#else
    // This is required for GL on Android, otherwise GPU memory leak occurs.
    eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
#endif

    // Disable VSync (for performance comparison).
    // glfwSwapInterval(0);

    return true;
}

void SwapChainGl::submit(const std::shared_ptr<CommandEncoder> &encoder) {
    if (encoder->submitted_) {
        Logger::error("Attempted to submit an encoder that's already been submitted!");
        return;
    }

    encoder->submitted_ = true;

    encoder->finish();
}

void SwapChainGl::present() {
#ifndef __ANDROID__
    glfwSwapBuffers((GLFWwindow *)glfw_window_);
#else
    eglSwapBuffers(egl_display_, egl_surface_);
#endif
}

} // namespace Pathfinder
