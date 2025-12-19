#include "window.h"

#include "render_api.h"

namespace Pathfinder {

#ifndef __ANDROID__
/// GLFW: whenever the window size changed (by OS or user) this callback function executes.
void framebuffer_resize_callback(GLFWwindow* glfw_window, int width, int height) {
    auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfw_window));

    if (window) {
        // Get the size of window decorations.
        int left, top, right, bottom;
        glfwGetWindowFrameSize(glfw_window, &left, &top, &right, &bottom);

        // TODO: consider window decorations.
        window->update_window_size(true, Vec2I(width, height));

        Logger::info("Window resized to physical" + window->get_physical_size().to_string() + ", logical" +
                     window->get_logical_size().to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void* Window::get_glfw_handle() const {
    return glfw_window_;
}
#endif

Window::Window(const Vec2I& size, void* window_handle) : physical_size_(size) {
#ifndef __ANDROID__
    glfw_window_ = window_handle;

    // Assign this to window user, so we can fetch it when window size changes.
    glfwSetWindowUserPointer((GLFWwindow*)glfw_window_, this);
    glfwSetFramebufferSizeCallback((GLFWwindow*)glfw_window_, framebuffer_resize_callback);
#endif
}

Vec2I Window::get_physical_size() const {
    return physical_size_;
}

Vec2I Window::get_logical_size() const {
    return (physical_size_.to_f32() / dpi_scaling_factor_).to_i32();
}

Vec2I Window::get_position() const {
    int xpos{}, ypos{};
#ifndef __ANDROID__
    glfwGetWindowPos((GLFWwindow*)glfw_window_, &xpos, &ypos);
#endif
    return {xpos, ypos};
}

bool Window::get_resize_flag() const {
    return just_resized_;
}

bool Window::is_minimized() const {
    return minimized_;
}

float Window::get_dpi_scaling_factor() const {
    return dpi_scaling_factor_;
}

void Window::set_dpi_scaling_factor(float scale) {
    dpi_scaling_factor_ = scale;
}

void Window::set_window_title(const std::string& title) const {
#ifndef __ANDROID__
    glfwSetWindowTitle((GLFWwindow*)glfw_window_, title.c_str());
#endif
}

void Window::update_window_size(bool resized, Vec2I physical_size) {
    just_resized_ = resized;
    physical_size_ = physical_size;
    minimized_ = physical_size_.area() == 0;
}

bool Window::should_close() {
#ifndef __ANDROID__
    return glfwWindowShouldClose((GLFWwindow*)glfw_window_);
#else
    return false;
#endif
}

void Window::hide() {
#ifndef __ANDROID__
    if (!hidden_) {
        glfwHideWindow((GLFWwindow*)glfw_window_);
        hidden_ = true;
    }
#endif
}

void Window::show() {
#ifndef __ANDROID__
    if (hidden_) {
        glfwShowWindow((GLFWwindow*)glfw_window_);
        hidden_ = false;
    }
#endif
}

} // namespace Pathfinder
