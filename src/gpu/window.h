#ifndef PATHFINDER_GPU_WINDOW_H
#define PATHFINDER_GPU_WINDOW_H

#include "device.h"
#include "queue.h"

namespace Pathfinder {

class SwapChain;

class Window {
public:
    explicit Window(Vec2I _size) : size(_size) {}

    virtual ~Window() = default;

    virtual std::shared_ptr<SwapChain> create_swap_chain(const std::shared_ptr<Device> &device) = 0;

    Vec2I get_size() const {
        return size;
    }

    bool get_resize_flag() const {
        return just_resized;
    }

    bool is_minimized() const {
        return minimized;
    }

    std::shared_ptr<SwapChain> swapchain;

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
