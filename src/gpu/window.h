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
    GLFWwindow *get_glfw_window() const;

    /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
    static void framebuffer_resize_callback(GLFWwindow *glfw_window, int width, int height);

    /// Process input events: query GLFW whether relevant keys are pressed/released this frame and react accordingly.
    void poll_events();

    bool should_close();
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_H
