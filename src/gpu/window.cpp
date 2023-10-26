#include "window.h"

namespace Pathfinder {

void Window::framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height) {
    auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));

    if (window) {
        window->just_resized = true;
        window->size = {width, height};
        window->minimized = window->size.area() == 0;

        Logger::info("Window resized to " + window->size.to_string());
    } else {
        Logger::error("glfwGetWindowUserPointer is NULL!");
    }
}

void Window::poll_events() {
    just_resized = false;

    glfwPollEvents();

    if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(glfw_window, true);
    }
}

bool Window::should_close() {
    return glfwWindowShouldClose(glfw_window);
}

GLFWwindow *Window::get_glfw_window() const {
    return glfw_window;
}

} // namespace Pathfinder
