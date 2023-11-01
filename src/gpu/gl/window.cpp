#include "window.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "swap_chain.h"

#if (defined(_WIN32) || defined(__linux__) || defined(__EMSCRIPTEN__))
    #if !defined(__ANDROID) && !defined(__EMSCRIPTEN__)
        #include <glad/gl.h>
    #endif

namespace Pathfinder {

    #ifndef __ANDROID__
WindowGl::WindowGl(const Vec2I &_size, GLFWwindow *window_handle) : Window(_size) {
    glfw_window = window_handle;

    // Assign this to window user, so we can fetch it when window size changes.
    glfwSetWindowUserPointer(glfw_window, this);
    glfwSetFramebufferSizeCallback(glfw_window, framebuffer_resize_callback);
}
    #else
WindowGl::WindowGl(const Vec2I &_size) : Window(_size) {}
    #endif

std::shared_ptr<SwapChain> WindowGl::create_swap_chain(const std::shared_ptr<Device> &device) {
    #ifndef __ANDROID__
    return std::make_shared<SwapChainGl>(size, glfw_window);
    #else
    return std::make_shared<SwapChainGl>(size);
    #endif
}

WindowGl::~WindowGl() {
    destroy();
}

void WindowGl::destroy() {
    #ifndef __ANDROID__
    if (glfw_window) {
        glfwDestroyWindow(glfw_window);
        glfw_window = nullptr;
    }
    #endif
}

} // namespace Pathfinder

#endif
