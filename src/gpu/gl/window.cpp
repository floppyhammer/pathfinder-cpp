#include "window.h"

#include <sstream>
#include <stdexcept>

#include "../../common/logger.h"
#include "swap_chain.h"

#if (defined(_WIN32) || defined(__linux__) || defined(__EMSCRIPTEN__))

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

    #ifndef __ANDROID__
void WindowGl::framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height) {
    auto window = reinterpret_cast<WindowGl *>(glfwGetWindowUserPointer(glfw_window));

    if (window) {
        window->just_resized = true;
        window->size = {width, height};
        window->minimized = window->size.area() == 0;

        Logger::info("Window resized to " + window->size.to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void WindowGl::poll_events() {
    just_resized = false;

    glfwPollEvents();

    if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(glfw_window, true);
    }
}

bool WindowGl::should_close() {
    return glfwWindowShouldClose(glfw_window);
}

void *WindowGl::get_raw_handle() const {
    return glfw_window;
}
    #endif

} // namespace Pathfinder

#endif
