#include "swap_chain.h"

#include "render_pass.h"

namespace Pathfinder {

#ifndef __ANDROID__
SwapChainGl::SwapChainGl(const Vec2I size, void *window_handle, PresentMode present_mode)
    : SwapChain(size, present_mode) {
    glfw_window_ = window_handle;

    glfwMakeContextCurrent((GLFWwindow *)glfw_window_);
    int interval = (present_mode == PresentMode::Immediate) ? 0 : 1;
    glfwSwapInterval(interval);

    command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

    render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear, "Swapchain Render Pass"));
}
#else
SwapChainGl::SwapChainGl(Vec2I size,
                         EGLDisplay egl_display,
                         EGLSurface egl_surface,
                         EGLContext egl_context,
                         PresentMode present_mode)
    : SwapChain(size, present_mode) {
    command_encoder_ = std::shared_ptr<CommandEncoderGl>(new CommandEncoderGl());

    render_pass_ = std::shared_ptr<RenderPassGl>(new RenderPassGl(AttachmentLoadOp::Clear, "Swapchain Render Pass"));

    egl_display_ = egl_display;
    egl_surface_ = egl_surface;
    egl_context_ = egl_context;

    eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    EGLint interval = (present_mode == PresentMode::Immediate) ? 0 : 1;
    eglSwapInterval(egl_display_, interval);
}
#endif

bool SwapChainGl::acquire_image() {
#ifndef __ANDROID__
    glfwMakeContextCurrent((GLFWwindow *)glfw_window_);
#else
    // This is required for GL on Android, otherwise GPU memory leak occurs.
    eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
#endif

    return true;
}

void SwapChainGl::submit(const std::shared_ptr<CommandEncoder> &encoder) {
    if (encoder->submitted_) {
        Logger::error("Attempted to submit an encoder that's already been submitted!");
        return;
    }

    encoder->submitted_ = true;

    encoder->prepare();
}

void SwapChainGl::present() {
#ifndef __ANDROID__
    glfwSwapBuffers((GLFWwindow *)glfw_window_);
#else
    eglSwapBuffers(egl_display_, egl_surface_);
#endif
}

} // namespace Pathfinder
