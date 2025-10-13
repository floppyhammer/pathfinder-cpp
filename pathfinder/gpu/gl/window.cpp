#include "window.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "swap_chain.h"

namespace Pathfinder {

#ifndef __ANDROID__
WindowGl::WindowGl(const Vec2I &size, GLFWwindow *window_handle) : Window(size, window_handle) {}
#else
WindowGl::WindowGl(const Vec2I &size, EGLDisplay egl_display, EGLSurface egl_surface, EGLContext egl_context)
    : Window(size) {
    egl_display_ = egl_display;
    egl_surface_ = egl_surface;
    egl_context_ = egl_context;
}
#endif

std::shared_ptr<SwapChain> WindowGl::get_swap_chain(const std::shared_ptr<Device> &device) {
    if (swapchain_) {
        return swapchain_;
    }

#ifndef __ANDROID__
    swapchain_ = std::make_shared<SwapChainGl>(get_physical_size(), glfw_window_);
#else
    swapchain_ = std::make_shared<SwapChainGl>(get_physical_size(), egl_display_, egl_surface_, egl_context_);
#endif

    return swapchain_;
}

void WindowGl::destroy() {
#ifndef __ANDROID__
    if (glfw_window_) {
        glfwDestroyWindow(glfw_window_);
        glfw_window_ = nullptr;
    }
#endif
}

} // namespace Pathfinder
