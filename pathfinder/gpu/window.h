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

    Vec2I get_physical_size() const;

    Vec2I get_logical_size() const;

    Vec2I get_position() const;

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

    float get_dpi_scaling_factor() const;

    void set_dpi_scaling_factor(float scale);

    void set_window_title(const std::string& title) const;

    uint8_t window_index{};

protected:
    Vec2I logical_size_;

    bool just_resized_ = false;
    bool fullscreen_ = false;
    bool minimized_ = false;
    bool hidden_ = false;

    float dpi_scaling_factor_ = 1.0f;

#ifndef __ANDROID__
    GLFWwindow* glfw_window_{};
#endif
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_WINDOW_H
