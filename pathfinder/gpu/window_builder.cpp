#include "window_builder.h"

#include "window.h"

#ifdef PATHFINDER_USE_VULKAN
    #include "vk/base.h"
#else
    #include "gl/base.h"
#endif

namespace Pathfinder {

std::shared_ptr<Window> WindowBuilder::get_primary_window() const {
    return primary_window_;
}

void WindowBuilder::poll_events() {
    // Reset window flags.
    {
        primary_window_->just_resized_ = false;

        for (auto w : sub_windows_) {
            if (!w.expired()) {
                w.lock()->just_resized_ = false;
            }
        }
    }

#ifndef __ANDROID__
    glfwPollEvents();
#endif
}

#ifndef __ANDROID__
GLFWwindow *WindowBuilder::glfw_window_init(const Vec2I &logical_size,
                                            const std::string &title,
                                            float &dpi_scaling_factor,
                                            GLFWwindow *shared_window) {
    #ifndef __EMSCRIPTEN__
    // Enable window resizing.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Hide window upon creation as we need to center the window before showing it.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Get monitor position (used to correctly center the window in a multi-monitor scenario).
    int monitor_count, monitor_x, monitor_y;
    GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);

    const GLFWvidmode *video_mode = glfwGetVideoMode(monitors[0]);
    glfwGetMonitorPos(monitors[0], &monitor_x, &monitor_y);

    // Get DPI scale.
    float dpi_scale_x, dpi_scale_y;
    glfwGetMonitorContentScale(monitors[0], &dpi_scale_x, &dpi_scale_y);
    assert(dpi_scale_x == dpi_scale_y);
    dpi_scaling_factor = dpi_scale_x;
    #endif

    #if defined(__linux__) || defined(_WIN32)
    auto physical_size = (logical_size.to_f32() * dpi_scaling_factor).to_i32();
    #else ifdef __APPLE__
    auto physical_size = logical_size;
    #endif

    auto glfw_window = glfwCreateWindow(physical_size.x, physical_size.y, title.c_str(), nullptr, shared_window);
    if (glfw_window == nullptr) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    #ifndef __EMSCRIPTEN__
    // Center the window.
    glfwSetWindowPos(glfw_window,
                     monitor_x + (video_mode->width - physical_size.x) / 2,
                     monitor_y + (video_mode->height - physical_size.y) / 2);

    // Show the window.
    glfwShowWindow(glfw_window);
    #endif

    return glfw_window;
}
#endif

} // namespace Pathfinder
