#ifndef PATHFINDER_WINDOW_BUILDER_H
#define PATHFINDER_WINDOW_BUILDER_H

#include "device.h"
#include "queue.h"

namespace Pathfinder {

class Window;

class WindowBuilder {
public:
    static std::shared_ptr<WindowBuilder> new_impl(const Vec2I &size);

    virtual std::shared_ptr<Window> create_window(const Vec2I &_size, const std::string &title) = 0;

    virtual void destroy_window(const std::shared_ptr<Window> &window) = 0;

    std::shared_ptr<Window> get_main_window() const;

    static GLFWwindow *common_glfw_window_init(const Vec2I &size,
                                               const std::string &title,
                                               GLFWwindow *shared_window = nullptr) {
#ifndef __ANDROID__
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

    virtual std::shared_ptr<Device> request_device() = 0;

    virtual std::shared_ptr<Queue> create_queue() = 0;

protected:
    std::shared_ptr<Window> main_window;
    std::vector<std::weak_ptr<Window>> sub_windows;
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_H
