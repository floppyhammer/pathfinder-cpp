#ifndef PATHFINDER_GPU_WINDOW_H
#define PATHFINDER_GPU_WINDOW_H

#include "driver.h"

namespace Pathfinder {

class Window {
public:
    explicit Window(Vec2I _size) : size(_size) {}

    static std::shared_ptr<Window> new_impl(DeviceType device_type, Vec2I _size);

    virtual std::shared_ptr<Driver> create_driver() = 0;

    virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) = 0;

    virtual void cleanup() = 0;

    Vec2I get_size() const {
        return size;
    }

    bool get_resize_flag() const {
        return just_resized;
    }

    bool get_minimized() const {
        return minimized;
    }

protected:
    GLFWwindow *glfw_window{};

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

            Logger::info("Window resized to " + window->size.to_string() + ".");
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
