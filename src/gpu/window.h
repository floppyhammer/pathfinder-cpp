#ifndef PATHFINDER_GPU_WINDOW_H
#define PATHFINDER_GPU_WINDOW_H

#include "device.h"
#include "queue.h"

namespace Pathfinder {

class Window {
public:
    explicit Window(Vec2I _size) : size(_size) {}

    static std::shared_ptr<Window> new_impl(Vec2I _size);

    virtual std::shared_ptr<Device> request_device() = 0;

    virtual std::shared_ptr<Queue> create_queue() = 0;

    virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) = 0;

    virtual void cleanup() = 0;

    Vec2I get_size() const {
        return size;
    }

    bool get_resize_flag() const {
        return just_resized;
    }

    bool is_minimized() const {
        return minimized;
    }

protected:
    inline void common_glfw_init() {
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

        glfw_window = glfwCreateWindow(size.x, size.y, "Pathfinder", nullptr, nullptr);
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

        // Assign this to window user, so we can fetch it when window size changes.
        glfwSetWindowUserPointer(glfw_window, this);
        glfwSetFramebufferSizeCallback(glfw_window, framebuffer_resize_callback);
#endif
    }

protected:
#ifndef __ANDROID__
    GLFWwindow *glfw_window{};
#endif

    Vec2I size;
    bool just_resized = false;
    bool minimized = false;

#ifndef __ANDROID__
public:
    inline GLFWwindow *get_glfw_window() const {
        return glfw_window;
    }

    /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
    static void framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height) {
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

    /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
    inline void poll_events() {
        just_resized = false;

        glfwPollEvents();

        if (glfwGetKey(glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(glfw_window, true);
        }
    }

    inline bool should_close() {
        return glfwWindowShouldClose(glfw_window);
    }
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_H
