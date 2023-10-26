#include "window_builder.h"

namespace Pathfinder {

std::shared_ptr<Window> WindowBuilder::get_main_window() const {
    return main_window;
}

GLFWwindow* WindowBuilder::common_glfw_window_init(const Vec2I& size,
                                                   const std::string& title,
                                                   GLFWwindow* shared_window) {
#ifndef __ANDROID__
    #ifndef __EMSCRIPTEN__
    // Enable window resizing.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Hide window upon creation as we need to center the window before showing it.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Get monitor position (used to correctly center the window in a multi-monitor scenario).
    int monitor_count, monitor_x, monitor_y;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

    const GLFWvidmode* video_mode = glfwGetVideoMode(monitors[0]);
    glfwGetMonitorPos(monitors[0], &monitor_x, &monitor_y);

    // Get DPI scale.
    float dpi_scale_x, dpi_scale_y;
    glfwGetMonitorContentScale(monitors[0], &dpi_scale_x, &dpi_scale_y);
    #endif

    auto glfw_window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, shared_window);
    if (glfw_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    #ifndef __EMSCRIPTEN__
    // Center the window.
    glfwSetWindowPos(glfw_window,
                     monitor_x + (video_mode->width - size.x) / 2,
                     monitor_y + (video_mode->height - size.y) / 2);

    // Show the window.
    glfwShowWindow(glfw_window);
    #endif

    return glfw_window;
#endif
}

} // namespace Pathfinder
