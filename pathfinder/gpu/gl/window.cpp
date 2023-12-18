#include "window.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "swap_chain.h"

#if (defined(_WIN32) || defined(__linux__) || defined(__EMSCRIPTEN__))

namespace Pathfinder {

    #ifndef __ANDROID__
WindowGl::WindowGl(const Vec2I &_size, GLFWwindow *window_handle) : Window(_size) {
    glfw_window_ = window_handle;

    // Assign this to window user, so we can fetch it when window size changes.
    glfwSetWindowUserPointer(glfw_window_, this);
    glfwSetFramebufferSizeCallback(glfw_window_, framebuffer_resize_callback);
}
    #else
WindowGl::WindowGl(const Vec2I &_size) : Window(_size) {}
    #endif

std::shared_ptr<SwapChain> WindowGl::get_swap_chain(const std::shared_ptr<Device> &device) {
    if (swapchain_) {
        return swapchain_;
    }

    #ifndef __ANDROID__
    swapchain_ = std::make_shared<SwapChainGl>(size_, glfw_window_);
    #else
    swapchain_ = std::make_shared<SwapChainGl>(size_);
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

    #ifndef __ANDROID__
void WindowGl::framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height) {
    auto window = reinterpret_cast<WindowGl *>(glfwGetWindowUserPointer(glfw_window));

    if (window) {
        window->just_resized_ = true;
        window->size_ = {width, height};
        window->minimized_ = window->size_.area() == 0;

        Logger::info("Window resized to " + window->size_.to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void WindowGl::poll_events() {
    just_resized_ = false;

    glfwPollEvents();

    if (glfwGetKey(glfw_window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(glfw_window_, true);
    }
}

bool WindowGl::should_close() {
    return glfwWindowShouldClose(glfw_window_);
}

void *WindowGl::get_raw_handle() const {
    return glfw_window_;
}
    #endif

} // namespace Pathfinder

#endif
