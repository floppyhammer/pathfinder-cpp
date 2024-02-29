#include "window.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "vk/base.h"
#else
    #include "gl/base.h"
#endif

namespace Pathfinder {

#ifdef __ANDROID__
Window::Window(const Vec2I& size) : size_(size) {}
#else
Window::Window(const Vec2I& size, GLFWwindow* window_handle) : size_(size), glfw_window_(window_handle) {
    // Assign this to window user, so we can fetch it when window size changes.
    glfwSetWindowUserPointer(glfw_window_, this);
    glfwSetFramebufferSizeCallback(glfw_window_, framebuffer_resize_callback);
}
#endif

Vec2I Window::get_size() const {
    return size_;
}

bool Window::get_resize_flag() const {
    return just_resized_;
}

bool Window::is_minimized() const {
    return minimized_;
}

#ifndef __ANDROID__
void Window::framebuffer_resize_callback(GLFWwindow* glfw_window, int width, int height) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

    if (window) {
        window->just_resized_ = true;
        window->size_ = {width, height};
        window->minimized_ = window->size_.area() == 0;

        Logger::info("Window resized to " + window->size_.to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void* Window::get_glfw_handle() const {
    return glfw_window_;
}

bool Window::should_close() {
    return glfwWindowShouldClose(glfw_window_);
}

void Window::hide() {
    glfwHideWindow(glfw_window_);
}

void Window::show() {
    glfwShowWindow(glfw_window_);
}

#endif

} // namespace Pathfinder
