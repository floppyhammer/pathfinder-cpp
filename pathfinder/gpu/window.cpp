#include "window.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "vk/base.h"
#else
    #include "gl/base.h"
#endif

namespace Pathfinder {

#ifdef __ANDROID__
Window::Window(const Vec2I& size) : logical_size_(size) {
}
#else
Window::Window(const Vec2I& size, GLFWwindow* window_handle) : logical_size_(size), glfw_window_(window_handle) {
    // Assign this to window user, so we can fetch it when window size changes.
    glfwSetWindowUserPointer(glfw_window_, this);
    glfwSetFramebufferSizeCallback(glfw_window_, framebuffer_resize_callback);
}
#endif

Vec2I Window::get_physical_size() const {
    return (logical_size_.to_f32() * dpi_scaling_factor_).to_i32();
}

Vec2I Window::get_logical_size() const {
    return logical_size_;
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
        window->logical_size_ = (Vec2F(width, height) / window->dpi_scaling_factor_).to_i32();
        window->minimized_ = window->logical_size_.area() == 0;

        Logger::info("Window resized to " + window->logical_size_.to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void* Window::get_glfw_handle() const {
    return glfw_window_;
}

float Window::get_dpi_scaling_factor() const {
    return dpi_scaling_factor_;
}

void Window::set_dpi_scaling_factor(float scale) {
    dpi_scaling_factor_ = scale;
}

bool Window::should_close() {
    return glfwWindowShouldClose(glfw_window_);
}

void Window::hide() {
    if (!hidden_) {
        glfwHideWindow(glfw_window_);
        hidden_ = true;
    }
}

void Window::show() {
    if (hidden_) {
        glfwShowWindow(glfw_window_);
        hidden_ = false;
    }
}

#endif

} // namespace Pathfinder
