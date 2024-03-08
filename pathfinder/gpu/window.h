#ifndef PATHFINDER_GPU_WINDOW_H
#define PATHFINDER_GPU_WINDOW_H

#include "device.h"
#include "queue.h"
#include "swap_chain.h"

struct GLFWwindow;

namespace Pathfinder {

class SwapChain;

class Window {
    friend class WindowBuilder;
    friend class WindowBuilderGl;

public:
#ifdef __ANDROID__
    explicit Window(const Vec2I& size);
#else
    explicit Window(const Vec2I& size, GLFWwindow* window_handle);
#endif

    virtual ~Window() = default;

    virtual void destroy() = 0;

    virtual std::shared_ptr<SwapChain> get_swap_chain(const std::shared_ptr<Device>& device) = 0;

    Vec2I get_size() const;

    void hide();

    void show();

    bool get_resize_flag() const;

    bool is_minimized() const;

    void* get_glfw_handle() const;

#ifndef __ANDROID__
    bool should_close();
#endif

    /// GLFW: whenever the window size changed (by OS or user) this callback function executes.
    static void framebuffer_resize_callback(GLFWwindow* glfw_window, int width, int height);

    std::shared_ptr<SwapChain> swapchain_;

protected:
    Vec2I size_;
    bool just_resized_ = false;
    bool minimized_ = false;
    bool hiden_ = false;

#ifndef __ANDROID__
    GLFWwindow* glfw_window_{};
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_H
