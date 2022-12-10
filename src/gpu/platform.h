#ifndef PATHFINDER_GPU_PLATFORM_H
#define PATHFINDER_GPU_PLATFORM_H

#include "driver.h"

namespace Pathfinder {

class Platform {
public:
    explicit Platform(Vec2I _window_size) : window_size(_window_size) {}

    static std::shared_ptr<Platform> new_impl(DeviceType device_type, Vec2I _window_size);

    virtual std::shared_ptr<Driver> create_driver() = 0;

    virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Driver> &driver) = 0;

    virtual void cleanup() = 0;

    Vec2I get_window_size() const {
        return window_size;
    }

    bool get_window_resize_flag() const {
        return window_just_resized;
    }

    bool get_window_minimized() const {
        return window_minimized;
    }

protected:
    GLFWwindow *window{};
    Vec2I window_size;
    bool window_just_resized = false;
    bool window_minimized = false;

#ifndef __ANDROID__
public:
    inline GLFWwindow *get_glfw_window() const {
        return window;
    }

    /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
    static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
        auto platform = reinterpret_cast<Platform *>(glfwGetWindowUserPointer(window));

        if (platform) {
            platform->window_just_resized = true;

            platform->window_size = {width, height};

            platform->window_minimized = platform->window_size.area() == 0;

            Logger::info("Window resized to " + platform->window_size.to_string() + ".");
        } else {
            Logger::error("glfwGetWindowUserPointer is NULL!");
        }
    }

    /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
    inline void poll_events() {
        window_just_resized = false;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_PLATFORM_H
