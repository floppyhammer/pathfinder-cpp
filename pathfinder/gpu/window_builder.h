#ifndef PATHFINDER_WINDOW_BUILDER_H
#define PATHFINDER_WINDOW_BUILDER_H

#include "device.h"
#include "queue.h"

#ifdef __ANDROID__
    #include <EGL/egl.h>
#endif

class ANativeWindow;
class GLFWwindow;

namespace Pathfinder {

class Window;

static const char *PRIMARY_WINDOW_TITLE = "Primary Window";

/// Window management.
class WindowBuilder {
public:
    static std::shared_ptr<WindowBuilder> new_impl(BackendType backend_type, const Vec2I &size);

    virtual ~WindowBuilder() = default;

    /// Wait for the swapchains to finish the current frame, then destroy them.
    /// Call this right after the render loop is stopped.
    virtual void stop_and_destroy_swapchains() {}

    /// Create a new sub-window.
    virtual uint8_t create_window(const Vec2I &size, const std::string &title) = 0;

    std::weak_ptr<Window> get_window(uint8_t window_index) const;

    float get_dpi_scaling_factor(uint8_t window_index) const;

    void set_dpi_scaling_factor(uint8_t window_index, float new_scale);

    virtual std::shared_ptr<Device> request_device() = 0;

    virtual std::shared_ptr<Queue> create_queue() = 0;

    void poll_events();

    void set_fullscreen(bool fullscreen);

protected:
#ifndef __ANDROID__
    static GLFWwindow *glfw_window_init(const Vec2I &logical_size,
                                        const std::string &title,
                                        float &dpi_scaling_factor,
                                        bool fullscreen,
                                        GLFWwindow *shared_window);
#else
    ANativeWindow *native_window_{};
#endif

    std::shared_ptr<Window> primary_window_;
    std::vector<std::shared_ptr<Window>> sub_windows_;

    bool primary_window_fullscreen_ = false;

    // Size before going fullscreen or being minimized.
    Vec2I reserved_window_physical_size_;

    Vec2I reserved_window_position_;
};

} // namespace Pathfinder

#endif // PATHFINDER_WINDOW_BUILDER_H
